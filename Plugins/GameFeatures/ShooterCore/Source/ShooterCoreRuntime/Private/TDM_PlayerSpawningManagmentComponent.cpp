// Copyright Epic Games, Inc. All Rights Reserved.

#include "TDM_PlayerSpawningManagmentComponent.h"

#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "GameModes/LyraGameState.h"
#include "Player/LyraPlayerStart.h"
#include "Teams/LyraTeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TDM_PlayerSpawningManagmentComponent)

class AActor;

UTDM_PlayerSpawningManagmentComponent::UTDM_PlayerSpawningManagmentComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

AActor* UTDM_PlayerSpawningManagmentComponent::OnChoosePlayerStart(AController* Player, TArray<ALyraPlayerStart*>& PlayerStarts)
{
	ULyraTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
	const int32 PlayerTeamId = TeamSubsystem->FindTeamFromObject(Player);

	//if (GEngine)
	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("chcking!, %d"), PlayerTeamId));

	// We should have a TeamId by now, but early login stuff before post login can try to do stuff, ignore it.
	if (!ensure(PlayerTeamId != INDEX_NONE))
	{
		return nullptr;
	}

	ALyraGameState* GameState = GetGameStateChecked<ALyraGameState>();

	ALyraPlayerStart* BestPlayerStart = nullptr;
	double MaxDistance = 0;
	ALyraPlayerStart* FallbackPlayerStart = nullptr;
	double FallbackMaxDistance = 0;

	//FGameplayTag MyTag = FGameplayTag::RequestGameplayTag(FName("Lyra.Player"));

	for (APlayerState* PS : GameState->PlayerArray)
	{
		const int32 TeamId = TeamSubsystem->FindTeamFromObject(PS);

		// We should have a TeamId by now...
		if (PS->IsOnlyASpectator() || !ensure(TeamId != INDEX_NONE))
		{
			continue;
		}
		// added by MK
		// TeamId of player defined in ULyraTeamCreationComponent::ServerChooseTeamForPlayer
		/*if (PlayerTeamId == 1) {
			for (ALyraPlayerStart* PlayerStart : PlayerStarts)
			{
				if (PlayerStart->GetGameplayTags().HasTag(MyTag) )
				{
					BestPlayerStart = PlayerStart;
					if (GEngine)
						GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("found!, %d"), TeamId));
				}
			}
		}*/

		// If the other player isn't on the same team, lets find the furthest spawn from them.
		//if (GEngine)
		//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("tt!, %d"), PlayerStarts.Num()));

		if (TeamId != PlayerTeamId)
		{
			for (ALyraPlayerStart* PlayerStart : PlayerStarts)
			{
				if (APawn* Pawn = PS->GetPawn())
				{
					const double Distance = PlayerStart->GetDistanceTo(Pawn);

					if (PlayerStart->IsClaimed())
					{
						if (FallbackPlayerStart == nullptr || Distance > FallbackMaxDistance)
						{
							FallbackPlayerStart = PlayerStart;
							FallbackMaxDistance = Distance;
						}
					}
					else if (PlayerStart->GetLocationOccupancy(Player) < ELyraPlayerStartLocationOccupancy::Full)
					{
						if (BestPlayerStart == nullptr || Distance > MaxDistance)
						{
							BestPlayerStart = PlayerStart;
							MaxDistance = Distance;
						}
					}
				}
			}
		}
	}

	if (BestPlayerStart)
	{
		return BestPlayerStart;
	}

	return FallbackPlayerStart;
}

void UTDM_PlayerSpawningManagmentComponent::OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation)
{
	
}
