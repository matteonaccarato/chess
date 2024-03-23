// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChessEnums.h"
// #include "BasePawn.h"
// #include "GameField.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

class ABasePawn;

USTRUCT(BlueprintType)
struct FTileStatus
{
	GENERATED_BODY()

	ABasePawn* Piece;
	bool EmptyFlag; // 1 => Empty, 0 => Occupied
	TArray<bool> AttackableFrom; // 0-cell => from WHITE, 1-cell => from BLACK
	TArray<ABasePawn*> WhoCanGo;
	EPawnColor PawnColor;
	EPawnType PawnType;
	int8 PlayerOwner;
};

UCLASS()
class CHESS_API ATile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATile();

	UStaticMeshComponent* GetStaticMeshComponent() const;

	void SetPawn(ABasePawn* TilePawn);
	ABasePawn* GetPawn() const;

	FString GetId() const;

	void SetLetterId(const FString TileLetter);
	FString GetLetterId() const;

	void SetNumberId(const int8 TileNumber);
	int8 GetNumberId() const;

	void SetTileStatus(const FTileStatus TileStatus);
	FTileStatus GetTileStatus() const;

	void SetPlayerOwner(const int32 P_Owner);
	int32 GetPlayerOwner() const;

	void SetGridPosition(const double InX, const double InY);
	FVector2D GetGridPosition() const;

	void ClearInfo();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* MaterialLight;

	UPROPERTY(EditAnywhere, Category="Materials")
	UMaterialInterface* MaterialDark;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* MaterialGreen;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterial* Material;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInstance* MaterialInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ABasePawn* Pawn;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString LetterId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int NumberId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FTileStatus Status;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 PlayerOwner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D TileGridPosition;

// public:	
	// Called every frame
	// virtual void Tick(float DeltaTime) override;

};
