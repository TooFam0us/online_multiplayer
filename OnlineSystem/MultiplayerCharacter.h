// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetworkedBullet.h"
#include "GenericStats.h"
#include "MultiplayerCharacter.generated.h"


UCLASS()
class AMultiplayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMultiplayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(ReplicatedUsing = OnRep_TakeDamage)
		float Health = 1.f;

	UPROPERTY(ReplicatedUsing = OnRep_SpendStamina)
		float Stamina = 1.f;




protected:

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class ANetworkedBullet> ProjectileClass;

	UFUNCTION(BlueprintCallable)
		void OnFire(FTransform Transform, FGenericStat ProjectileStat, FLinearColor Color);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_OnFire(FTransform Transform, FGenericStat ProjectileStat, FLinearColor Color);
	bool Server_OnFire_Validate(FTransform Transform, FGenericStat ProjectileStat, FLinearColor Color);
	void Server_OnFire_Implementation(FTransform Transform, FGenericStat ProjectileStat, FLinearColor Color);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void Multi_OnFire(FTransform Transform, FGenericStat ProjectileStat, FLinearColor Color);
	bool Multi_OnFire_Validate(FTransform Transform, FGenericStat ProjectileStat, FLinearColor Color);
	void Multi_OnFire_Implementation(FTransform Transform, FGenericStat ProjectileStat, FLinearColor Color);

public:
	UFUNCTION()
		void OnRep_TakeDamage();

	UFUNCTION()
	void OnRep_SpendStamina();

	UFUNCTION(BlueprintCallable)
	void SetHealth(float amount);

	UFUNCTION(BlueprintCallable)
	void SetStamina(float amount);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		float GetHealth() { return Health; };

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetStamina() { return Stamina; };

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	UFUNCTION(BlueprintCallable)
	void ReceiveDamage(float DamageTaken);

	UFUNCTION(BlueprintCallable)
	void SpendStamina(float StaminaSpent);

};
