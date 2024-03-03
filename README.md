# CHESS



quando il re può essere mangiato (king_tile == attackable_from),
bool IsCheck = true
in gamemode => uniche valid move (a meno del coprire la traiettoria) sono il re che scappa; se tutte sono attackable from opponent, checkmate
(poi gestire possibilità di coprire e mangiare con altre pedine quello che mette sotto scacco)

=>

IsValidMove() {
	if (IsCheck())
		simulateMove
		ShowPossibleMoves(all black)
		if king_tile == attackable_from => still in check
}
=> ma crea loop infinito




AGGIORNARE DA IPHONE NOTES E NOTION
GameMode.cpp@353

settare ogni cella attaccabile come attaccabile (evitare che il re ci vada dentro)
se tutte le mosse possibili del re sono attackable => check mate
e non riesco a mangiare quegli attaccanti


How To Create UMG UI in Unreal Engine - Animating Widgets 
https://youtu.be/SD66UgyHiMM?si=ZoCKFO_R1YV-gRLk


il pawn promotion è obbligatorio o l'utente può non farlo?


#### Main Feature
- [ ] Check and CheckMate (look at iphone notes)
- [ ] Pawn Promotion
- [ ] Reset
- [ ] MiniMax Player
- [ ] Replay

#### TODO:
toimplement NON è POSSIBILE MANGIARE I RE
- [ ] Leggere regolamento (e implementare regole particolari di mangiare e proteggere re)
- [ ] => Mettere punti cardinali corretti (anche rispetto a neri => NORTH implica andare indietro)
- [ ] RandomPlayer / actually to implement choose and eat opponent's pawns
- [ ] id in ciascuna tile
- [ ] Pawns / MaxNumberSteps => to split by X and by Y
- [ ] ogni tipo di pedina ha una classe per identitificare codice e mosse possibili (base pawn => team, eligible_moves)
- [ ] aggiungere controlli nei parametri per != nullptr
- [ ] calcolo ischeck 1° for superfluo se gli passo un pawn (vuol dire che è sottoscacco e voglio provare a liberarmi)
- [ ] std::pair va bene o unreal si offende
- [ ] king se mangia una pedina e poi può essere mangiato, non viene notato come mossa non lecita (perché appunto c'è una pedina sulla tile che diventerebbe attackable)


#### TO REVIEW (if already implemented)
- [ ] GameMode@69 mettere la classe come parametro della funzione
- [ ] Movimento pedine: in classe padre attributo "possible_movements" (definire x e y di arrivo e metodo: "L", "forward", ...)
- [ ] commentare funzioni come ho fatto in php f1 project (title, param, type, return)
- [ ] in random player => toglliere case epawntype@101, gestire tutti i movimenti nelle direzioni cardinali, eat flag dipende da se in newx e newy è NON occupata (occupata da bianco)
- [ ] aggiornare vettore tile/pedine ad ogni spostamento


#### EXTRA
- [ ] Fare menu in cui si sceglie il colore delle tile, delle pedine e del tavolo da gioco (sfondo, piuttosto che vuoto)
- [ ] togliere texture dal lato dei cubi
- [ ] Material dei blueprint pezzi dinamico => non 6 e 6 ma solo 6
- [ ] Transizione da menu a tavolo da gioco, con luce che incrementa man mano
- [ ] sito web (how to play chess)
- [ ] Look at iphone notes and notion
- [ ] Movement pawn sound
- [ ] Commenti avversario (tipo chess.com)
- [ ] Elenco mosse fatte stile chess.com
- [ ] Caricare specifica configurazione di partita (per allenarti) digitando dove posizionare le varie pedine
- [ ] Account per registrare uno storico delle partite fatte con i risultati e uno ?screenshot? della board finale
- [ ] Animazione dello spostamento delle pedine