// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/ReplayManager.h"

ReplayManager::ReplayManager() {}
ReplayManager::~ReplayManager() {}

/*
 * Function: AddToReplay
 * ----------------------------
 *   Add the last move to the replay scrollbox.
 *	  The piece is taken as argument, while the previous tile is taken from the attributes of the GameMode
 *	  (PreviousGridPosition: FVector2D)
 *
 *	 Pawn				const ABasePawn*		The pawn which has been moved
 *	 EatFlag			const bool = false		If another piece has been captured
 *	 PawnPromotionFlag	const bool = false		If a pawn promotion has been happened
 */
void ReplayManager::AddToReplay(AChess_GameMode* GameMode, const ABasePawn* Pawn, const bool EatFlag, const bool PawnPromotionFlag)
{
	if (GameMode)
	{
		FString MoveStr = ComputeMoveName(GameMode, Pawn, EatFlag, PawnPromotionFlag);

		// Update RecordMoves
		GameMode->RecordMoves.Add(MoveStr);

		// Update Replay widget content
		UWorld* World = GameMode->GetWorld();
		UScrollBox* ScrollBox = Pawn->GetColor() == EPawnColor::WHITE ?
			Cast<UScrollBox>(GameMode->ReplayWidget->GetWidgetFromName(TEXT("scr_Replay_white")))
			: Cast<UScrollBox>(GameMode->ReplayWidget->GetWidgetFromName(TEXT("scr_Replay_black")));
		if (World && ScrollBox && GameMode->ButtonWidgetRef)
		{
			UUserWidget* WidgetBtn = CreateWidget(World, GameMode->ButtonWidgetRef);
			if (WidgetBtn)
			{
				UTextBlock* BtnText = Cast<UTextBlock>(WidgetBtn->GetWidgetFromName(TEXT("txtBlock")));
				if (BtnText)
				{
					BtnText->SetText(FText::FromString(FString::Printf(TEXT("%d. %s"), GameMode->MoveCounter, *MoveStr)));
				}

				FWidgetTransform Transform;
				Transform.Angle = 180;
				WidgetBtn->SetRenderTransform(Transform);
				UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(ScrollBox->AddChild(WidgetBtn));
				ScrollSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
				ScrollSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
				ScrollBox->ScrollToEnd();
			}
		}	
	}

}

/*
 * Function: ComputeMoveName const
 * ----------------------------
 *   Generate the algebraic notation of the last move.
 *		The piece is taken as argument, while the previous tile is taken from the attributes of the GameMode
 *			(PreviousGridPosition: FVector2D)
 *
 *   Pawn				const ABasePawn*		The pawn which has been moved
 *	 EatFlag			const bool = false		If another piece has been captured
 *	 PawnPromotionFlag	const bool = false		If a pawn promotion has been happened
 */
FString ReplayManager::ComputeMoveName(AChess_GameMode* GameMode, const ABasePawn* Pawn, const bool EatFlag, const bool PawnPromotionFlag)
{
	FString MoveStr = TEXT("");
	if (GameMode)
	{
		FVector2D GridPosition = Pawn->GetGridPosition();
		ATile* Tile = GameMode->GField->GetTileArray()[GridPosition[0] * GameMode->GField->Size + GridPosition[1]];
		ATile* PreviousTile = GameMode->GField->GetTileArray()[GameMode->PreviousGridPosition[0] * GameMode->GField->Size + GameMode->PreviousGridPosition[1]];
		bool IsPawn = Pawn->GetType() == EPawnType::PAWN;

		FString StartTileStr = TEXT("");
		int8 DeltaY = Pawn->GetGridPosition()[1] - GameMode->PreviousGridPosition[1];
		if (Pawn && Pawn->GetType() == EPawnType::KING && FMath::Abs(DeltaY) > 1 && !Tile->GetId().IsEmpty())
		{
			// Castling handling
			MoveStr = FMath::Sign(DeltaY) > 0 ? "0-0" : "0-0-0";
		}
		else
		{
			// Handling of more than one piece can move on the tile
			for (auto& Piece : Tile->GetTileStatus().WhoCanGo)
			{
				if (Pawn->GetType() == Piece->GetType()
					&& Pawn->GetColor() == Piece->GetColor()
					&& Pawn->GetPieceNum() != Piece->GetPieceNum())
				{
					if (GameMode->PreviousGridPosition[1] == Piece->GetGridPosition()[1])
						StartTileStr += FString::FromInt(PreviousTile->GetNumberId());
					else
						StartTileStr += PreviousTile->GetLetterId().ToLower();
				}
			}

			if (Pawn && !Tile->GetId().IsEmpty())
			{
				// Create move name strings (algebraic notation, e.g. Rb8#)
				MoveStr = ((IsPawn || PawnPromotionFlag) ? TEXT("") : Pawn->GetId()) +
					StartTileStr +
					(((IsPawn || PawnPromotionFlag) && EatFlag && StartTileStr == TEXT("")) ? PreviousTile->GetLetterId().ToLower() : TEXT("")) +
					(EatFlag ? TEXT("x") : TEXT("")) +
					Tile->GetId().ToLower() +
					(PawnPromotionFlag ? (TEXT("=") + Pawn->GetId()) : TEXT("")) +
					((GameMode->CheckFlag != EPawnColor::NONE && GameMode->MatchStatus == EMatchResult::NONE) ? TEXT("+") : TEXT("")) +
					(GameMode->MatchStatus == EMatchResult::WHITE || GameMode->MatchStatus == EMatchResult::BLACK ? TEXT("#") : TEXT(""));
			}
		}
	}
	return MoveStr;
}