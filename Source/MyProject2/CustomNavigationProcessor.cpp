// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomNavigationProcessor.h"
#include "CustomMassObstacleAvoidanceTrait.h"
#include "CustomAvoidanceProcessor.h"

#include "MassCommonUtils.h"
#include "MassCommandBuffer.h"
#include "MassCommonFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavigationSubsystem.h"
#include "MassSimulationLOD.h"
#include "MassMovementTypes.h"
#include "MassMovementFragments.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "Engine/World.h"

#define UNSAFE_FOR_MT 0
#define MOVEMENT_DEBUGDRAW 0	// Set to 1 to see heading debugdraw
//----------------------------------------------------------------------//
//  UMassNavigationObstacleGridProcessor
//----------------------------------------------------------------------//
UCustomMassNavigationObstacleGridProcessor::UCustomMassNavigationObstacleGridProcessor()
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);

}

void UCustomMassNavigationObstacleGridProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::InitializeInternal(Owner, EntityManager);

	CustomNavigationSubsystem = UWorld::GetSubsystem<UCustomMassNavigationSubsystem>(Owner.GetWorld());
}


void UCustomMassNavigationObstacleGridProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	FMassEntityQuery BaseEntityQuery;
	BaseEntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	BaseEntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	BaseEntityQuery.AddRequirement<FCustomMassNavigationObstacleGridCellLocationFragment>(EMassFragmentAccess::ReadWrite);
	BaseEntityQuery.AddSubsystemRequirement<UCustomMassNavigationSubsystem>(EMassFragmentAccess::ReadWrite);

	AddToGridEntityQuery = BaseEntityQuery;
	AddToGridEntityQuery.AddRequirement<FMassAvoidanceColliderFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	AddToGridEntityQuery.AddTagRequirement<FMassOffLODTag>(EMassFragmentPresence::None);
	AddToGridEntityQuery.AddTagRequirement<FMassInNavigationObstacleGridTag>(EMassFragmentPresence::None);
	AddToGridEntityQuery.RegisterWithProcessor(*this);

	UpdateGridEntityQuery = BaseEntityQuery;
	UpdateGridEntityQuery.AddRequirement<FMassAvoidanceColliderFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	UpdateGridEntityQuery.AddTagRequirement<FMassOffLODTag>(EMassFragmentPresence::None);
	UpdateGridEntityQuery.AddTagRequirement<FMassInNavigationObstacleGridTag>(EMassFragmentPresence::All);
	UpdateGridEntityQuery.RegisterWithProcessor(*this);

	RemoveFromGridEntityQuery = BaseEntityQuery;
	RemoveFromGridEntityQuery.AddTagRequirement<FMassOffLODTag>(EMassFragmentPresence::All);
	RemoveFromGridEntityQuery.AddTagRequirement<FMassInNavigationObstacleGridTag>(EMassFragmentPresence::All);
	RemoveFromGridEntityQuery.RegisterWithProcessor(*this);
}

void UCustomMassNavigationObstacleGridProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	PointCloud2D pc;

	if (!CustomNavigationSubsystem->isQueueEmpty())
	{
		EntityManager.Defer().DestroyEntities(CustomNavigationSubsystem->GetZombiesToKill());
		CustomNavigationSubsystem->EraseZombiesToKill();
	}

	AddToGridEntityQuery.ForEachEntityChunk(Context, [this, &pc, &EntityManager](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();
			TConstArrayView<FTransformFragment> LocationList = Context.GetFragmentView<FTransformFragment>();

			for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
			{
				// Add to the grid
				const auto entity = Context.GetEntity(EntityIndex);
				const FVector NewPos = LocationList[EntityIndex].GetTransform().GetLocation();
				pc.pts.emplace_back(PointCloud2D::Point{ NewPos.X, NewPos.Y, entity });
				Context.Defer().AddTag<FMassInNavigationObstacleGridTag>(entity);
			}
		});

	UpdateGridEntityQuery.ForEachEntityChunk(Context, [this, &pc, &EntityManager](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();
			TConstArrayView<FTransformFragment> LocationList = Context.GetFragmentView<FTransformFragment>();

			for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
			{
				// Add to the grid
				const auto entity = Context.GetEntity(EntityIndex);
				const FVector NewPos = LocationList[EntityIndex].GetTransform().GetLocation();
				pc.pts.emplace_back(PointCloud2D::Point{ NewPos.X, NewPos.Y, entity });
			}
		});

	RemoveFromGridEntityQuery.ForEachEntityChunk(Context, [this, &EntityManager](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();

			for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
			{
				Context.Defer().RemoveTag<FMassInNavigationObstacleGridTag>(Context.GetEntity(EntityIndex));
			}
		});

	CustomNavigationSubsystem->SetPoints(pc);
}

//----------------------------------------------------------------------//
//  UMassNavigationObstacleRemoverProcessor
//----------------------------------------------------------------------//
UCustomMassNavigationObstacleRemoverProcessor::UCustomMassNavigationObstacleRemoverProcessor()
	: EntityQuery(*this)
{
	ObservedTypes.Add(FCustomMassNavigationObstacleGridCellLocationFragment::StaticStruct());
	ObservedOperations = EMassObservedOperationFlags::Remove;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
}

void UCustomMassNavigationObstacleRemoverProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FCustomMassNavigationObstacleGridCellLocationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UCustomMassNavigationSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UCustomMassNavigationObstacleRemoverProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			auto& HashGrid = Context.GetMutableSubsystemChecked<UCustomMassNavigationSubsystem>().GetObstacleGridMutable();
			const int32 NumEntities = Context.GetNumEntities();
			const TArrayView<FCustomMassNavigationObstacleGridCellLocationFragment> AvoidanceObstacleCellLocationList = Context.GetMutableFragmentView<FCustomMassNavigationObstacleGridCellLocationFragment>();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				FMassNavigationObstacleItem ObstacleItem;
				ObstacleItem.Entity = Context.GetEntity(i);
				//HashGrid.Remove(ObstacleItem, AvoidanceObstacleCellLocationList[i].CellLoc);
			}
		});
}

#undef UNSAFE_FOR_MT