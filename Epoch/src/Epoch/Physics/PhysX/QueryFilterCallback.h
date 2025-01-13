#pragma once
#include <PxQueryFiltering.h>
#include "PhysXShape.h"

namespace Epoch
{
	class QueryFilterCallback : public physx::PxQueryFilterCallback
	{
	public:
		/**
		\brief This filter callback is executed before the exact intersection test if	PxQueryFlag::ePREFILTER flag was set.

		\param[in] filterData		custom filter data specified as the query's filterData.data		parameter.
		\param[in] shape			A shape that has not yet passed the exact intersection test.
		\param[in] actor			The shape's actor.
		\param[in,out] queryFlags	scene query flags from the query's function call (only flags	from PxHitFlag::eMODIFIABLE_FLAGS bitmask can be modified)
		\return the updated type for this hit  (see #PxQueryHitType)
		*/
		physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override
		{
			return physx::PxQueryHitType::Enum::eNONE;
		}

		/**
		\brief This filter callback is executed if the exact intersection test returned true and PxQueryFlag::ePOSTFILTER flag was set.

		\param[in] filterData	custom filter data of the query
		\param[in] hit			Scene query hit information. faceIndex member is not valid for overlap queries. For sweep and raycast queries the hit information can be cast to #PxSweepHit and #PxRaycastHit respectively.
		\param[in] shape		Hit shape
		\param[in] actor		Hit actor
		\return the updated hit type for this hit  (see #PxQueryHitType)
		*/
		physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit, const physx::PxShape* shape, const physx::PxRigidActor* actor) override
		{
			return physx::PxQueryHitType::Enum::eNONE;
		}
	}
}
