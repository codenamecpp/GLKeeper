/*
SoLoud audio engine
Copyright (c) 2013-2018 Jari Komppa

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

#include "soloud.h"

namespace SoLoud
{
	QueueInstance::QueueInstance(Queue *aParent)
	{
		mParent = aParent;
		mFlags |= PROTECTED;
	}
	
	unsigned int QueueInstance::getAudio(float *aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize)
	{
		if (mParent->mCount == 0)
		{
			return 0;			
		}
		unsigned int copycount = aSamplesToRead;
		unsigned int copyofs = 0;
		while (copycount && mParent->mCount)
		{
			int readcount = mParent->mSource[mParent->mReadIndex]->getAudio(aBuffer + copyofs, copycount, aBufferSize);
			copyofs += readcount;
			copycount -= readcount;
			if (mParent->mSource[mParent->mReadIndex]->hasEnded())
			{
				delete mParent->mSource[mParent->mReadIndex];
				mParent->mSource[mParent->mReadIndex] = 0;
				mParent->mReadIndex = (mParent->mReadIndex + 1) % SOLOUD_QUEUE_MAX;
				mParent->mCount--;
				mLoopCount++;
			}
		}
		return copyofs;
	}

	bool QueueInstance::hasEnded()
	{
		return mLoopCount != 0 && mParent->mCount == 0;
	}

	QueueInstance::~QueueInstance()
	{
		if(mParent)
			mParent->clearInstanceInternal();
	}

	Queue::Queue()
	{
		mQueueHandle = 0;
		mInstance = 0;
		mReadIndex = 0;
		mWriteIndex = 0;
		mCount = 0;
		int i;
		for (i = 0; i < SOLOUD_QUEUE_MAX; i++)
			mSource[i] = nullptr;
	}
	
	QueueInstance * Queue::createInstance()
	{
		if (mInstance)
		{
			stop();
			delete mInstance;
			mInstance = 0;
		}
		mInstance = new QueueInstance(this);
		return mInstance;
	}

	void Queue::findQueueHandle()
	{
		// Find the channel the queue is playing on to calculate handle..
		int i;
		for (i = 0; mQueueHandle == 0 && i < (signed)mSoloud->mHighestVoice; i++)
		{
			if (mSoloud->mVoice[i] == mInstance)
			{
				mQueueHandle = mSoloud->getHandleFromVoice_internal(i);
			}
		}
	}

	result Queue::play(AudioSource &aSound)
	{
		if (!mSoloud)
		{
			return INVALID_PARAMETER;
		}
	
		// it is not clear, why we would need this
		//findQueueHandle();
		//if (mQueueHandle == 0)
		//	return INVALID_PARAMETER;

		if (mCount >= SOLOUD_QUEUE_MAX)
			return OUT_OF_MEMORY;

		if (!aSound.mAudioSourceID)
		{
			aSound.mAudioSourceID = mSoloud->mAudioSourceID;
			mSoloud->mAudioSourceID++;
		}

		SoLoud::AudioSourceInstance *instance = aSound.createInstance();

		if (instance == 0)
		{
			return OUT_OF_MEMORY;
		}
		instance->init(aSound, 0);
		instance->mAudioSourceID = aSound.mAudioSourceID;

		mSoloud->lockAudioMutex_internal();
		mSource[mWriteIndex] = instance;
		mWriteIndex = (mWriteIndex + 1) % SOLOUD_QUEUE_MAX;
		mCount++;
		mSoloud->unlockAudioMutex_internal();

		return SO_NO_ERROR;
	}

	void Queue::stop_queue()
	{
		// empty queue. this is not very efficient, since the mutex is locked in each iteration. oh well...
		while(getQueueCount() > 0)
			pop();

		stop();
		mInstance = 0; // instance was deleted from stop()
	}

	result Queue::pop()
	{
		if (!mSoloud)
		{
			return UNKNOWN_ERROR;
		}

		mSoloud->lockAudioMutex_internal();
		if(mCount == 0)
		{
			mSoloud->unlockAudioMutex_internal();
			return SO_NO_ERROR; // queue empty, no future sound available to remove
		}
		if( 0 == mWriteIndex ) // cant use modulo operation due to temporary negative numbers
			mWriteIndex = SOLOUD_QUEUE_MAX -1;
		else
			mWriteIndex -= 1;
		delete mSource[mWriteIndex];
		mSource[mWriteIndex] = nullptr;
		mCount--;
		mSoloud->unlockAudioMutex_internal();

		return SO_NO_ERROR;
	}


	unsigned int Queue::getQueueCount()
	{
		unsigned int count;
		mSoloud->lockAudioMutex_internal();
		count = mCount;
		mSoloud->unlockAudioMutex_internal();
		return count;
	}

	bool Queue::isCurrentlyPlaying(AudioSource &aSound)
	{
		if (mSoloud == 0 || mCount == 0 || aSound.mAudioSourceID == 0)
			return false;
		mSoloud->lockAudioMutex_internal();
		bool res = mSource[mReadIndex]->mAudioSourceID == aSound.mAudioSourceID;
		mSoloud->unlockAudioMutex_internal();
		return res;
	}

	result Queue::setParamsFromAudioSource(AudioSource &aSound)
	{
		mChannels = aSound.mChannels;
		mBaseSamplerate = aSound.mBaseSamplerate;

	    return SO_NO_ERROR;
	}
	
	result Queue::setParams(float aSamplerate, unsigned int aChannels)
	{
	    if (aChannels < 1 || aChannels > MAX_CHANNELS)
	        return INVALID_PARAMETER;
		mChannels = aChannels;
		mBaseSamplerate = aSamplerate;
	    return SO_NO_ERROR;
	}


	bool Queue::hasEnded()
	{
		if(mInstance)
			return mInstance->hasEnded();
		else
			return true;
	}

	void Queue::clearInstanceInternal()
	{
		if(mSoloud->mInsideAudioThreadMutex)
		{
			// we cannot use stop_queue() here to clean things up, this at this point the mutex is already locked and stop_queue will attempt to lock it again
			while(mCount > 0)
			{
				if( 0 == mWriteIndex ) // cant use modulo operation due to temporary negative numbers
					mWriteIndex = SOLOUD_QUEUE_MAX -1;
				else
					mWriteIndex -= 1;
				delete mSource[mWriteIndex];
				mSource[mWriteIndex] = nullptr;
				mCount--;
			}
			mInstance = 0;
		}
		else
		{
			stop_queue();
		}
	}
};
