#include "stdafx.h"
#include "CreatureAction_FaceTarget.h"
#include "Creature.h"
#include "MapUtils.h"

CreatureAction_FaceTarget::CreatureAction_FaceTarget()
    : CreatureAction(eCreatureAction_FaceTarget)
{
}

void CreatureAction_FaceTarget::Configure(Creature* creature, const MapPoint2D& mapTile)
{
    CreatureAction::Configure(creature);
    mTargetMapTile.emplace(mapTile);
}

void CreatureAction_FaceTarget::OnRecycle()
{
    CreatureAction::OnRecycle();
    mTargetMapTile.reset();
}

void CreatureAction_FaceTarget::HandleEnterAction()
{
    Locomotion& locomotion = GetCreature().GetLocomotion();
    locomotion.ClearGoals();

    // face to target tile
    if (mTargetMapTile)
    {
        const MapPoint2D currTile = GetCreature().GetTilePosition();
        const MapPoint2D targetTile = *mTargetMapTile;

        const int horz_delta = (targetTile.x - currTile.x);
        const int vert_delta = (targetTile.y - currTile.y);
        bool onStraightLine = (horz_delta == 0) != (vert_delta == 0);
        if (onStraightLine)
        {
            const glm::vec2 creaturePosition = GetCreature().GetPosition2d();
            const glm::vec2 faceTargetPosition = creaturePosition + glm::vec2{horz_delta * 1.0f, vert_delta * 1.0f};
            locomotion.OrientToPoint(faceTargetPosition, GetCreature().GetTransform());
        }
        else
        {
            glm::vec2 centerPoint = MapUtils::ComputeTileCenter2d(targetTile);
            locomotion.OrientToPoint(centerPoint, GetCreature().GetTransform());
        }
        return;
    }
}

void CreatureAction_FaceTarget::HandleLeaveAction()
{
    Locomotion& locomotion = GetCreature().GetLocomotion();
    locomotion.ClearGoals();
}

void CreatureAction_FaceTarget::HandleUpdateLogic(float stepDeltaTime)
{
    if (IsCancellationRequested())
    {
        SetActionResult(CreatureAction::eResult_Cancelled);
        return;
    }

    Locomotion& locomotion = GetCreature().GetLocomotion();
    if (!locomotion.HasGoals())
    {
        SetActionResult(CreatureAction::eResult_Success);
        return;
    }
}

void CreatureAction_FaceTarget::HandleResumeAction(eCreatureAction subActionId, eResult subActionResult)
{

}
