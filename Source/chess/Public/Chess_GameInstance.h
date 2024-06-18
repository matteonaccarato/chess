// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Chess_GameInstance.generated.h"

UENUM()
enum class EMatchMode : uint8
{
	HUMAN_RANDOM, 
	HUMAN_MINIMAX_BASE,
	RANDOM_RANDOM,
	RANDOM_MINIMAX_BASE,
	MINIMAX_BASE_MINIMAX_BASE,
	MINIMAX_BASE_MINIMAX_PESTO
};

/**
 * 
 */
UCLASS()
class CHESS_API UChess_GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	/* UI MESSAGES */
	static constexpr char* DRAW = "DRAW";
	/* HUMAN */
	static constexpr char* HUMAN		= "YOU";
	static constexpr char* HUMAN_TURN   = "YOU";
	static constexpr char* HUMAN_WIN    = "You WON!";
	static constexpr char* HUMAN_DEFEAT = "You LOST!";
	/* RANDOM */
	static constexpr char* RANDOM        = "AI | Random";
	static constexpr char* RANDOM_1      = "AI | Random (1)";
	static constexpr char* RANDOM_2      = "AI | Random (2)";
	static constexpr char* RANDOM_TURN   = "AI (Random)";
	static constexpr char* RANDOM_WIN_1  = "AI ";
	static constexpr char* RANDOM_WIN_2  = " WON";
	static constexpr char* RANDOM_DEFEAT = "AI (Random) LOST!";
	/* MINIMAX */
	static constexpr char* MINIMAX		  = "AI | MiniMax";
	static constexpr char* MINIMAX_1      = "AI | MiniMax (1)";
	static constexpr char* MINIMAX_2	  = "AI | MiniMax (2)";
	static constexpr char* MINIMAX_P	  = "AI | MiniMax (Pesto)";
	static constexpr char* MINIMAX_TURN   = "AI (MiniMax)";
	static constexpr char* MINIMAX_WIN_1  = "AI ";
	static constexpr char* MINIMAX_WIN_2  = " WON";
	static constexpr char* MINIMAX_DEFEAT = "AI (MiniMax) LOST!";


	/* ATTRIBUTES */
	UPROPERTY(EditAnywhere)
	FString TextPlayer_1 = "Player 1";

	UPROPERTY(EditAnywhere)
	FString TextPlayer_2 = "Player 2";

	UPROPERTY(EditAnywhere)
	int32 ScorePlayer_1 = 0;

	UPROPERTY(EditAnywhere)
	int32 ScorePlayer_2 = 0;

	UPROPERTY(EditAnywhere)
	int32 DrawsCounter = 0;

	UPROPERTY(EditAnywhere)
	int32 GamesCounter = 0;

	UPROPERTY(EditAnywhere)
	FString StopwatchStr = "00:00";

	UPROPERTY(EditAnywhere)
	FString CurrentTurnMessage = "Current Player";

	UPROPERTY(EditAnywhere)
	EMatchMode MatchMode;


	/* METHODS */
	/* PLAYER 1 */
	UFUNCTION(BlueprintCallable)
	FString GetPlayerText_1() const;
	void SetPlayerText_1(FString Text);

	UFUNCTION(BlueprintCallable)
	int32 GetScorePlayer_1() const;
	void SetScorePlayer_1(int32 Score);
	void IncrementScorePlayer_1();


	/* PLAYER 2 */
	UFUNCTION(BlueprintCallable)
	FString GetPlayerText_2() const;
	void SetScorePlayer_2(int32 Score);
	void SetPlayerText_2(FString Text);

	UFUNCTION(BlueprintCallable)
	int32 GetScorePlayer_2() const;
	void IncrementScorePlayer_2();


	/* DRAWS COUNTER */
	UFUNCTION(BlueprintCallable)
	int32 GetDrawsCounter() const;
	void SetDrawsCounter(int32 Counter);
	void IncrementDrawsCounter();


	/* GAMES COUNTER */
	UFUNCTION(BlueprintCallable)
	int32 GetGamesCounter() const;
	void SetGamesCounter(int32 Counter);
	void IncrementGamesCounter();


	/* STOPWATCH */
	UFUNCTION(BlueprintCallable)
	FString GetStopwatchStr() const;
	int Minutes = 0;
	int Seconds = 0;
	void SetStopwatchStr(FString Text);
	void IncrementStopwatch();


	/* TURN */
	UFUNCTION(BlueprintCallable)
	FString GetTurnMessage() const;

	UFUNCTION(BlueprintCallable)
	void SetTurnMessage(FString Message);


	/* MATCH MODE */
	UFUNCTION(BlueprintCallable)
	EMatchMode GetMatchMode() const;

	UFUNCTION(BlueprintCallable)
	void SetMatchMode(EMatchMode Mode);
};
