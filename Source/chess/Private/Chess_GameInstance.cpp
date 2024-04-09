// Fill out your copyright notice in the Description page of Project Settings.


#include "Chess_GameInstance.h"

void UChess_GameInstance::IncrementScoreHumanPlayer()
{
	ScoreHumanPlayer++;
}

void UChess_GameInstance::IncrementScoreAiPlayer()
{
	ScoreAiPlayer++;
}

int32 UChess_GameInstance::GetScoreHumanPlayer() const
{
	return ScoreHumanPlayer;
}

int32 UChess_GameInstance::GetScoreAiPlayer() const
{
	return ScoreAiPlayer;
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
