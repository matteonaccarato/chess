// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Chess_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class CHESS_API UChess_GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	int32 ScoreHumanPlayer = 0;

	UPROPERTY(EditAnywhere)
	int32 ScoreAiPlayer = 0;

	UPROPERTY(EditAnywhere)
	FString CurrentTurnMessage = "Current Player";

	void IncrementScoreHumanPlayer();
	void IncrementScoreAiPlayer();

	UFUNCTION(BlueprintCallable)
	int32 GetScoreHumanPlayer();

	UFUNCTION(BlueprintCallable)
	int32 GetScoreAiPlayer();

	UFUNCTION(BlueprintCallable)
	FString GetTurnMessage();

	UFUNCTION(BlueprintCallable)
	void SetTurnMessage(FString Message);
};
