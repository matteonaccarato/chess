// Fill out your copyright notice in the Description page of Project Settings.


#include "Chess_GameInstance.h"

/* PLAYER 1 */
FString UChess_GameInstance::GetPlayerText_1() const
{
	return TextPlayer_1;
}

void UChess_GameInstance::SetPlayerText_1(FString Text)
{
	TextPlayer_1 = Text;
}

int32 UChess_GameInstance::GetScorePlayer_1() const
{
	return ScorePlayer_1;
}

void UChess_GameInstance::IncrementScorePlayer_1()
{
	ScorePlayer_1++;
}


/* PLAYER 2 */
FString UChess_GameInstance::GetPlayerText_2() const
{
	return TextPlayer_2;
}

void UChess_GameInstance::SetPlayerText_2(FString Text)
{
	TextPlayer_2 = Text;
}

int32 UChess_GameInstance::GetScorePlayer_2() const
{
	return ScorePlayer_2;
}

void UChess_GameInstance::IncrementScorePlayer_2()
{
	ScorePlayer_2++;
}


/* GAMES COUNTER */
void UChess_GameInstance::IncrementGamesCounter()
{
	GamesCounter++;
}

FString UChess_GameInstance::GetStopwatchStr() const
{
	return StopwatchStr;
}

void UChess_GameInstance::SetStopwatchStr(FString Text)
{
	StopwatchStr = Text;
}

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

int32 UChess_GameInstance::GetGamesCounter() const
{
	return GamesCounter;
}

void UChess_GameInstance::SetGamesCounter(int32 Counter)
{
	GamesCounter = Counter;
}


/* TURN */
FString UChess_GameInstance::GetTurnMessage() const
{
	return CurrentTurnMessage;
}

void UChess_GameInstance::SetTurnMessage(FString Message)
{
	CurrentTurnMessage = Message;
}


/* MATCH MODE */
EMatchMode UChess_GameInstance::GetMatchMode() const
{
	return MatchMode;
}

void UChess_GameInstance::SetMatchMode(EMatchMode Mode)
{
	MatchMode = Mode;
}
