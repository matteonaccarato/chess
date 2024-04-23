// Fill out your copyright notice in the Description page of Project Settings.


#include "Chess_GameInstance.h"

/* PLAYER 1 */
FString UChess_GameInstance::GetPlayerText_1() const { return TextPlayer_1; }
void UChess_GameInstance::SetPlayerText_1(FString Text) { TextPlayer_1 = Text; } 
int32 UChess_GameInstance::GetScorePlayer_1() const { return ScorePlayer_1; }
void UChess_GameInstance::SetScorePlayer_1(int32 Score) { ScorePlayer_1 = Score;  }
void UChess_GameInstance::IncrementScorePlayer_1() { ScorePlayer_1++; }

/* PLAYER 2 */
FString UChess_GameInstance::GetPlayerText_2() const { return TextPlayer_2; }
void UChess_GameInstance::SetPlayerText_2(FString Text) { TextPlayer_2 = Text; }
int32 UChess_GameInstance::GetScorePlayer_2() const { return ScorePlayer_2; }
void UChess_GameInstance::SetScorePlayer_2(int32 Score) { ScorePlayer_2 = Score; }
void UChess_GameInstance::IncrementScorePlayer_2() { ScorePlayer_2++; }

/* DRAWS COUNTER */
int32 UChess_GameInstance::GetDrawsCounter() const { return DrawsCounter; }
void UChess_GameInstance::SetDrawsCounter(int32 Counter) { DrawsCounter = Counter; }
void UChess_GameInstance::IncrementDrawsCounter() { DrawsCounter++; }

/* GAMES COUNTER */
void UChess_GameInstance::IncrementGamesCounter() { GamesCounter++; }
int32 UChess_GameInstance::GetGamesCounter() const { return GamesCounter; }
void UChess_GameInstance::SetGamesCounter(int32 Counter) { GamesCounter = Counter; }

/* STOPWATCH */
FString UChess_GameInstance::GetStopwatchStr() const { return StopwatchStr; }
void UChess_GameInstance::SetStopwatchStr(FString Text) { StopwatchStr = Text; }
void UChess_GameInstance::IncrementStopwatch()
{
	Seconds++;
	if (Seconds >= 60)
	{
		Minutes++;
		Seconds = 0;
	}

	StopwatchStr = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
}


/* TURN */
FString UChess_GameInstance::GetTurnMessage() const { return CurrentTurnMessage; }
void UChess_GameInstance::SetTurnMessage(FString Message) { CurrentTurnMessage = Message; }

/* MATCH MODE */
EMatchMode UChess_GameInstance::GetMatchMode() const { return MatchMode; }
void UChess_GameInstance::SetMatchMode(EMatchMode Mode) { MatchMode = Mode; }