// Fill out your copyright notice in the Description page of Project Settings.


#include "PRAT_WukongESkillAction.h"
#include "ProjectR/Character/PRCharacterBase.h"
#include "ProjectR/GA/PRGA_Skill.h"
#include "ProjectR/TAGS/PRGameplayTag.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "ProjectR/Character/PRCharacterPlayer.h"

#include <AbilitySystemComponent.h>
#include <TimerManager.h>
#include <Abilities/GameplayAbility.h>


UPRAT_WukongESkillAction::UPRAT_WukongESkillAction(const FObjectInitializer& objectinitializer): Super(objectinitializer)
{
	bTickingTask=true;
}

void UPRAT_WukongESkillAction::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	LOG_N(LogTemp, TEXT("틱테스크도는중"));

	UpdateCharacterRotationToMouse();
	UpdateCharacterMoveToMouse(DeltaTime);
}

void UPRAT_WukongESkillAction::Activate()
{
	Super::Activate();
	bTickingTask=true;
	FName StartSection;
	if (UAbilitySystemComponent* ASC = CurrentOwningAbility->GetAbilitySystemComponentFromActorInfo())
	{
		ASC->PlayMontage(CurrentOwningAbility, CurrentOwningAbility->GetCurrentActivationInfo(), UtilDataLoad::GetCharcacterSkillTypeData(Cast<APRCharacterBase>(Cast<UPRGA_Skill>(CurrentOwningAbility)->GetActorInfo().AvatarActor.Get()), *Cast<UPRGA_Skill>(CurrentOwningAbility)->GetInputKey()).SkillAnimMontage, 1.0f, StartSection);
	}
	//SpawnLightning();
	//GetWorld()->GetTimerManager().SetTimer(LightningTimerHandle, this, &UPRAT_WukongESkillAction::SpawnLightning, 0.5f, true);
	//Cast<UPRGA_Skill>(CurrentOwningAbility)->EndGameAblity.AddUObject(this, &UPRAT_WukongESkillAction::SkillCancleOrEndSet);
}

// void UPRAT_WukongESkillAction::CompleteActionSkillTask()
// {
// }

void UPRAT_WukongESkillAction::UpdateCharacterRotationToMouse()
{
	APlayerController* PlayerController = Cast<APlayerController>(targetcharacter->GetController());
	if (PlayerController)
	{
		FHitResult HitResult;
		PlayerController->GetHitResultUnderCursor(ECC_GameTraceChannel2, false, HitResult);
		FVector MouseLocation = HitResult.Location;
		FVector PlayerLocation = targetcharacter->GetActorLocation();
		FVector MouseDirection = MouseLocation - PlayerLocation;
		MouseDirection.Z = 0; // 평면에서 회전만 고려
		if (!MouseDirection.IsZero())
		{
			FRotator NewRotation = FRotationMatrix::MakeFromX(MouseDirection).Rotator();
			if (targetcharacter->GetLocalRole() == ROLE_AutonomousProxy)
				targetcharacter->GetController()->ClientSetRotation(NewRotation, 0.f);
		}
		else
		{
			//	UE_LOG(LogTemp, Warning, TEXT("MouseDirection is Zero. No rotation applied."));
		}
	}
}
void UPRAT_WukongESkillAction::UpdateCharacterMoveToMouse(float DeltaSeconds)
{
	
		FVector ForwardDirection = targetcharacter->GetActorForwardVector();

		// Normalize the forward direction to avoid excessive speed
		ForwardDirection.Normalize();

		// Calculate movement input (for example, you can scale by a movement speed factor)
		float MovementSpeed = 600.0f; // Adjust this value to your desired speed
		FVector MovementInput = ForwardDirection * MovementSpeed * DeltaSeconds;

		// Move the character in the forward direction
		targetcharacter->AddMovementInput(ForwardDirection, MovementSpeed * DeltaSeconds);
}


void UPRAT_WukongESkillAction::SpawnLightning()
{
	if (CurrentOwningAbility == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("CurrentOwningAbility is null"));
		return;
	}

	TArray<FOverlapResult> overlapResults;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(UABTA_Trace), false, CurrentOwningAbility->GetAvatarActorFromActorInfo());
	FVector SkillLocation = CurrentOwningAbility->GetAvatarActorFromActorInfo()->GetActorLocation();
	bool bShowDebug = true;
	for (auto overlap : SkillTypeData.SkillOverlapType)
	{
		if (overlap.SphereOverlap.Active)
		{
			LOG_N(LogTemp, TEXT("SphereOverlap"));
			bool enter = UtilOverlap::SphereOverlapByProfile(overlapResults, GetWorld(), SkillLocation, overlap.SphereOverlap.InRadius, FName(TEXT("Pawn")), Params, bShowDebug, overlap.SphereOverlap.InDebugColor, overlap.SphereOverlap.InDebugTime);
			if (enter)
			{
				LOG_N(LogTemp, TEXT("충돌처리 성공"));
			}
		}
		if (overlap.BoxOverlap.Active)
		{
			LOG_N(LogTemp, TEXT("BoxOverlap"));
			bool enter = UtilOverlap::BoxOverlapByProfile(overlapResults, GetWorld(), SkillLocation, overlap.BoxOverlap.InRot, overlap.BoxOverlap.InBoxHalfExtent, FName(TEXT("Pawn")), Params, bShowDebug, overlap.SphereOverlap.InDebugColor, overlap.BoxOverlap.InDebugTime);
			if (enter)
			{
				LOG_N(LogTemp, TEXT("충돌처리 성공"));
			}
		}
	}

	if (AActor* OwnerActor = CurrentOwningAbility->GetOwningActorFromActorInfo())
	{
		FVector Location = OwnerActor->GetActorLocation();
		SpawnParticle();
		UE_LOG(LogTemp, Warning, TEXT("번개가 %s 위치에 생성되었습니다."), *Location.ToString());
	}
}

void UPRAT_WukongESkillAction::SpawnParticle()
{
	LOG_N(LogTemp, TEXT("파티클 생성"));

	UAbilitySystemComponent* SourceASC = CurrentOwningAbility->GetAbilitySystemComponentFromActorInfo();
	AActor* OwnerActor = CurrentOwningAbility->GetOwningActorFromActorInfo();
	// 파티클을 실행시킬 위치와 방향 설정
	FVector SpawnLocation = OwnerActor->GetActorLocation();
	FRotator SpawnRotation = OwnerActor->GetActorRotation();

	// Gameplay Cue 파라미터 설정
	FGameplayCueParameters CueParams;
	CueParams.SourceObject = OwnerActor;
	CueParams.Location = SpawnLocation;

	// Gameplay Cue 실행
	SourceASC->ExecuteGameplayCue(PRTAG_GAMEPLAYCUE_WUKONG_QSKILL, CueParams);
}
void UPRAT_WukongESkillAction::SkillCancleOrEndSet()
{
	if (UAbilitySystemComponent* ASC = CurrentOwningAbility->GetAbilitySystemComponentFromActorInfo())
	{
		// 현재 능력을 사용하여 재생할 몽타주 데이터를 가져옵니다.
		UAnimMontage* MontageToPlay = UtilDataLoad::GetCharcacterSkillTypeData(
			Cast<APRCharacterBase>(Cast<UPRGA_Skill>(CurrentOwningAbility)->GetActorInfo().AvatarActor.Get()), 
			*Cast<UPRGA_Skill>(CurrentOwningAbility)->GetInputKey()
		).SkillAnimMontage;
		ASC->StopMontageIfCurrent(*MontageToPlay);
		//
		GetWorld()->GetTimerManager().PauseTimer(LightningTimerHandle);
		
		if(APRCharacterPlayer* Character = Cast<APRCharacterPlayer>(CurrentOwningAbility->GetAvatarActorFromActorInfo()))
		{
			Character->SetMousetoChangeCharacterRotate(false);
			FRotator NewRotation(0.0f, 0.0f, 0.0f);
			if (Character->GetController()->GetLocalRole() == ROLE_AutonomousProxy)
				Character->GetController()->ClientSetRotation(NewRotation);
			Character->SetMousetoCharacterMovement(false);
		}
	}
}


