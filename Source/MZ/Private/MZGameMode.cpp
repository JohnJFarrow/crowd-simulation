// Copyright Epic Games, Inc. All Rights Reserved.

#include "MZGameMode.h"
#include "MZCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMZGameMode::AMZGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		//DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
