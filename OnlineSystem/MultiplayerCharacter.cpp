// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

// Sets default values
AMultiplayerCharacter::AMultiplayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMultiplayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}



// Called every frame
void AMultiplayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMultiplayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMultiplayerCharacter::OnFire(FTransform Transform, FGenericStat ProjectileStat, FLinearColor Color)
{
	if (ProjectileClass != NULL)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawning on local client"));
		// Spawn the projectile at the muzzle.
		// This is the same as the server spawn, but we do it here so that the client can see the projectile immediately.
		// If we don't do this, the client will not see the projectile until the server tells it about it.
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ANetworkedBullet* Projectile = GetWorld()->SpawnActorDeferred<ANetworkedBullet>(this->ProjectileClass, Transform, this, NULL, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (Projectile)
		{
			Projectile->Init(ProjectileStat, Color);
			Projectile->FinishSpawning(Transform);
		}

		if (!HasAuthority())
		{// On CLIENT
			UE_LOG(LogTemp, Warning, TEXT("Firing from client"));
			Server_OnFire(Transform, ProjectileStat, Color);
		}
		else
		{// On SERVER
			UE_LOG(LogTemp, Warning, TEXT("Firing from server"));
			Multi_OnFire(Transform, ProjectileStat, Color);
		}
	}

}

bool AMultiplayerCharacter::Server_OnFire_Validate(FTransform Transform, FGenericStat ProjectileStat, FLinearColor Color)
{
	return true;
}

void AMultiplayerCharacter::Server_OnFire_Implementation(FTransform Transform, FGenericStat ProjectileStat, FLinearColor Color)
{
	Multi_OnFire(Transform, ProjectileStat, Color);
}

bool AMultiplayerCharacter::Multi_OnFire_Validate(FTransform Transform, FGenericStat ProjectileStat, FLinearColor Color)
{
	return true;
}

void AMultiplayerCharacter::Multi_OnFire_Implementation(FTransform Transform, FGenericStat ProjectileStat, FLinearColor Color)
{
	if (!IsLocallyControlled())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ANetworkedBullet* Projectile = GetWorld()->SpawnActorDeferred<ANetworkedBullet>(this->ProjectileClass, Transform, this, NULL, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (Projectile)
		{
			Projectile->Init(ProjectileStat, Color);
			Projectile->FinishSpawning(Transform);
		}
	}


}


void AMultiplayerCharacter::SetHealth(float amount)
{
	if (HasAuthority())
	{
		Health = amount;

		MARK_PROPERTY_DIRTY_FROM_NAME(AMultiplayerCharacter, Health, this);
	}
}

void AMultiplayerCharacter::SetStamina(float amount)
{
	if (HasAuthority())
	{
		Stamina = amount;

		MARK_PROPERTY_DIRTY_FROM_NAME(AMultiplayerCharacter, Stamina, this);
	}
}

void AMultiplayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	SharedParams.RepNotifyCondition = REPNOTIFY_Always;
	DOREPLIFETIME_WITH_PARAMS_FAST(AMultiplayerCharacter, Health, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(AMultiplayerCharacter, Stamina, SharedParams);
}

void AMultiplayerCharacter::OnRep_TakeDamage()
{
	UE_LOG(LogTemp, Warning, TEXT("On rep Take Damage, healt: %d"), int(Health));
}

void AMultiplayerCharacter::OnRep_SpendStamina()
{
	UE_LOG(LogTemp, Warning, TEXT("On rep Spend Stamina, Stamina: %d"), int(Stamina));
}

void AMultiplayerCharacter::ReceiveDamage(float DamageTaken)
{
	if (HasAuthority())
	{
		Health -= DamageTaken;
		MARK_PROPERTY_DIRTY_FROM_NAME(AMultiplayerCharacter, Health, this);

	}

}

void AMultiplayerCharacter::SpendStamina(float StaminaSpent)
{
	if (HasAuthority())
	{
		Stamina -= StaminaSpent;
		MARK_PROPERTY_DIRTY_FROM_NAME(AMultiplayerCharacter, Stamina, this);
	}
}

