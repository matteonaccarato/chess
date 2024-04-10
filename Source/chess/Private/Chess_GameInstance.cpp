// Fill out your copyright notice in the Description page of Project Settings.


#include "Chess_GameInstance.h"

void UChess_GameInstance::IncrementScorePlayer_1()
{
	ScorePlayer_1++;
}

void UChess_GameInstance::IncrementScorePlayer_2()
{
	ScorePlayer_2++;
}

int32 UChess_GameInstance::GetScorePlayer_1() const
{
	return ScorePlayer_1;
}

int32 UChess_GameInstance::GetScorePlayer_2() const
{
	return ScorePlayer_2;
}

FString UChess_GameInstance::GetTurnMessage() const
{
	return CurrentTurnMessage;
}

void UChess_GameInstance::SetTurnMessage(FString Message)
{
	CurrentTurnMessage = Message;
}

EMatchMode UChess_GameInstance::GetMatchMode() const
{
	return MatchMode;
}

void UChess_GameInstance::SetMatchMode(EMatchMode Mode)
{
	MatchMode = Mode;
}
