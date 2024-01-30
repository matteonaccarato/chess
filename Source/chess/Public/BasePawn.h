// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BasePawn.generated.h"

UCLASS()
class CHESS_API ABasePawn : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasePawn();

	/* void SetTileId(const FString TileId);
	FString GetTileId() const; */

	void SetGridPosition(const double InX, const double InY);
	FVector2D GetGridPosition() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/* UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString TileId; */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D TileGridPosition;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void SelfDestroy();
};
