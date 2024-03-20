# CHESS


partita patta 
	=> king vs king
	=> king non può essere mangiato ma tutte le tile intorno a lui sono attaccabili
	=> la stessa configurazione di partita si ripete 3 o 5 volte (tramite hash o controllo sequenziale?)

Arrocco (da verificare se soddisfa le condizioni di non passare, essere, andare in scacco)
En Passant



NOMENCLATURA per arrocco da aggiungere



How To Create UMG UI in Unreal Engine - Animating Widgets 
https://youtu.be/SD66UgyHiMM?si=ZoCKFO_R1YV-gRLk


il pawn promotion è obbligatorio o l'utente può non farlo?

bottoni legno:
	- <a href="https://www.freepik.com/free-vector/wooden-buttons-different-shapes-vector-illustrations-set-play-stop-check-star-buttons-with-wood-texture-progress-bar-white-background-user-interface-concept-game-app-website-design_24645050.htm">Image by pch.vector</a> on Freepik

#### Main Feature
- [?] Check and CheckMate (look at iphone notes)
- [X] Reset
- [ ] MiniMax Player
- [X] Replay
- [X] Pawn Promotion
- [ ] Arrocco / En Passant

#### TODO:
segnalazione di elementi null tramite UE_LOG()
Nomenclatura mosse:
	- pawn promotion
	- scacco matto
	- esito partita (vittoria/sconfitta/pareggio)
	- BUG nomenclatura mossa => se tolgo da scacco, rimane comunque il +
- [ ] Leggere regolamento (e implementare regole particolari di mangiare e proteggere re)
- [ ] aggiungere controlli nei parametri per != nullptr
- [ ] calcolo ischeck 1° for superfluo se gli passo un pawn (vuol dire che è sottoscacco e voglio provare a liberarmi)
- [ ] std::pair va bene o unreal si offende
- [ ] se king si posiziona tale che vuole mangiare l'altro king => situazione da gestire come non consentita (non la gestisco perché appena trovo king sotto attacco, faccio break)
- [ ] quando si vedono le possibili mosse (cambio material) e clicco su un replay, le mosse possibili rimangono visibili



#### BUGS
- [ ] Nomenclatura quando sotto scacco matto non funzionante
- [ ] Save board status quando scacco matto (salva due mosse in una (fatto video con telefono))

#### TO REVIEW (if already implemented)
- [ ] in random player => toglliere case epawntype@101, gestire tutti i movimenti nelle direzioni cardinali, eat flag dipende da se in newx e newy è NON occupata (occupata da bianco)
- [ ] Sostituire north sout, con forward backward



#### EXTRA
- [ ] Rinominare la classe ABasePawn in AChessPiece (Pawn è solo pedone)
- [ ] MEttere tutte le stringhe in TEXT("...")
- [ ] vedere bene quali metodi possono essere definiti costanti (const)
- [ ] al posto di const Class* => const Class&
- [ ] Fare menu in cui si sceglie il colore delle tile, delle pedine e del tavolo da gioco (sfondo, piuttosto che vuoto)
- [ ] Material dei blueprint pezzi dinamico => non 6 e 6 ma solo 6
- [ ] Transizione da menu a tavolo da gioco, con luce che incrementa man mano
- [ ] sito web (how to play chess)
- [ ] Movement pawn sound
- [ ] Commenti avversario (tipo chess.com)
- [ ] Caricare specifica configurazione di partita (per allenarti) digitando dove posizionare le varie pedine
- [ ] Account per registrare uno storico delle partite fatte con i risultati e uno ?screenshot? della board finale
- [ ] Animazione dello spostamento delle pedine

- [ ] FUNZIONI CHE RITORNANO VALORI PER IDENTIFICARE ERRORI ?

- [ ] cercare modo efficiente scacco e salvataggio partita (anche se memorizzando solo posizione hai tempo di ricostruzione della partita in O(1))
- [ ] REPLAY => va bene così, o conviene fare man mano dallo stato iniziale convertendo #id mossa a mossa sul campo ?
- [ ] cambiare EatFlag => in CaptureFlag