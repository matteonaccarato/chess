// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/ReplayManager.h"

ReplayManager::ReplayManager() {}
ReplayManager::~ReplayManager() {}

/*
 * Function: AddToReplay
 * ----------------------------
 * Add the last move to the replay scrollbox.
 * The piece is taken as argument, while the previous tile is taken from the attributes of the GameMode
 *
 * @param GameMode			const AChess_GameMode*	Gamemode to refer to
 * @param Piece				const ABasePiece*		The piece which has been moved
 * @param EatFlag			const bool = false		If another piece has been captured
 * @param PawnPromotionFlag	const bool = false		If a pawn promotion has been happened
 */
void ReplayManager::AddToReplay(AChess_GameMode* GameMode, const ABasePiece* Piece, const bool EatFlag, const bool PawnPromotionFlag)
{
	if (GameMode)
	{
		FString MoveStr = ComputeMoveName(GameMode, Piece, EatFlag, PawnPromotionFlag);

		// Update record moves history
		GameMode->RecordMoves.Add(MoveStr);

		// Update Replay widget content
		UWorld* World = GameMode->GetWorld();
		UScrollBox* ScrollBox = Piece->GetColor() == EPieceColor::WHITE ?
			Cast<UScrollBox>(GameMode->ReplayWidget->GetWidgetFromName(SCROLLBOX_WHITE_NAME)) :
			Cast<UScrollBox>(GameMode->ReplayWidget->GetWidgetFromName(SCROLLBOX_BLACK_NAME));

		if (World && ScrollBox && GameMode->ButtonWidgetRef)
		{
			UUserWidget* WidgetBtn = CreateWidget(World, GameMode->ButtonWidgetRef);
			if (WidgetBtn)
			{
				UTextBlock* BtnText = Cast<UTextBlock>(WidgetBtn->GetWidgetFromName(WIDGET_BUTTON_NAME));
				if (BtnText)
					BtnText->SetText(FText::FromString(FString::Printf(TEXT("%d. %s"), GameMode->MoveCounter, *MoveStr)));

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
 * Function: ComputeMoveName
 * ----------------------------
 * Generate the algebraic notation of the last move.
 * The piece is taken as argument, while the previous tile is taken from the attributes of the GameMode
 *
 * @param GameMode			AChess_GameMode*		GameMode to refer to
 * @param Piece				const ABasePiece*		The piece which has been moved
 * @param EatFlag			const bool = false		If another piece has been captured
 * @param PawnPromotionFlag	const bool = false		If a pawn promotion has been happened
 * 
 * @return					FString					Move name
 */
FString ReplayManager::ComputeMoveName(const AChess_GameMode* GameMode, const ABasePiece* Piece, const bool EatFlag, const bool PawnPromotionFlag)
{
	FString MoveStr = TEXT("");
	if (GameMode)
	{
		FVector2D GridPosition = Piece->GetGridPosition();
		ATile* Tile = GameMode->GField->TileArray[GridPosition[0] * GameMode->GField->Size + GridPosition[1]];
		ATile* PreviousTile = GameMode->GField->TileArray[GameMode->PreviousGridPosition[0] * GameMode->GField->Size + GameMode->PreviousGridPosition[1]];
		bool IsPawn = Piece->GetType() == EPieceType::PAWN;

		FString StartTileStr = TEXT("");
		int8 DeltaY = Piece->GetGridPosition()[1] - GameMode->PreviousGridPosition[1];
		if (Piece && Piece->GetType() == EPieceType::KING && FMath::Abs(DeltaY) > 1 && !Tile->GetId().IsEmpty())
		{
			// Castling handling
			MoveStr = FMath::Sign(DeltaY) > 0 ? SHORT_CASTLING : LONG_CASTLING;
		}
		else
		{
			// Handling of more than one piece can move on the tile
			for (const auto& PieceTmp : Tile->GetTileStatus().WhoCanGo)
			{
				if (Piece->GetType() == PieceTmp->GetType()
					&& Piece->GetColor() == PieceTmp->GetColor()
					&& Piece->GetPieceNum() != PieceTmp->GetPieceNum())
				{
					if (GameMode->PreviousGridPosition[1] == PieceTmp->GetGridPosition()[1])
						StartTileStr += FString::FromInt(PreviousTile->GetNumberId());
					else
						StartTileStr += PreviousTile->GetLetterId().ToLower();
				}
			}

			if (Piece && !Tile->GetId().IsEmpty())
			{
				// Create move name strings (algebraic notation, e.g. Rb8#)
				MoveStr = ((IsPawn || PawnPromotionFlag) ? TEXT("") : Piece->GetId()) +
					StartTileStr +
					(((IsPawn || PawnPromotionFlag) && EatFlag && StartTileStr == TEXT("")) ? PreviousTile->GetLetterId().ToLower() : TEXT("")) +
					(EatFlag ? TEXT("x") : TEXT("")) +
					Tile->GetId().ToLower() +
					(PawnPromotionFlag ? (TEXT("=") + Piece->GetId()) : TEXT("")) +
					((GameMode->CheckFlag != EPieceColor::NONE && GameMode->MatchStatus == EMatchResult::NONE) ? TEXT("+") : TEXT("")) +
					(GameMode->MatchStatus == EMatchResult::WHITE || GameMode->MatchStatus == EMatchResult::BLACK ? TEXT("#") : TEXT(""));
			}
		}
	}

	return MoveStr;
}