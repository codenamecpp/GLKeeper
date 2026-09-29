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

#ifndef SOLOUD_QUEUE_H
#define SOLOUD_QUEUE_H

#include "soloud.h"

#define SOLOUD_QUEUE_MAX 32

namespace SoLoud
{
	class Queue;

	// TODO: only a single Instance per queue is allowed. This should be enforced by code, e.g. combine both classes into a single one
	class QueueInstance : public AudioSourceInstance
	{
		Queue *mParent;
	public:
		QueueInstance(Queue *aParent);
		virtual unsigned int getAudio(float *aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize);
		virtual bool hasEnded();
		virtual ~QueueInstance();
	};

	class Queue : public AudioSource
	{
	public:
		Queue();
		virtual QueueInstance *createInstance();
		
		// Play sound through the queue
		result play(AudioSource &aSound);
		void stop_queue(); // stops playback and empties the queue

		// Deletes the most recently added sound of the queue
		// sounds that are currently playing are not popped
		// it is safe to not check the count before attempting a pop (this avoids a mutex lock)
		result pop();

        // Number of audio sources queued for replay
        unsigned int getQueueCount();
		
		// Is this audio source currently playing?
		bool isCurrentlyPlaying(AudioSource &aSound);
		
		// Set params by reading them from an audio source
		result setParamsFromAudioSource(AudioSource &aSound);
		
		// Set params manually
		result setParams(float aSamplerate, unsigned int aChannels = 2);

		bool hasEnded();
		
		void clearInstanceInternal(); // called by QueueInstance dtor
	public:
		// mCount: if there are no sounds: mCount=0; if one sound is currently playing and no other sound is queued: mCount=1; if a sound is playing and another one is queued: mCount=2
	    unsigned int mReadIndex, mWriteIndex, mCount;
	    AudioSourceInstance *mSource[SOLOUD_QUEUE_MAX];
		QueueInstance *mInstance;
		handle mQueueHandle; // unclear purpose. this variable is never used
		void findQueueHandle();
		
	};
};

#endif