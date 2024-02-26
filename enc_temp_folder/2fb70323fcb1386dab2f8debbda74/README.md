# CHESS


AGGIORNARE DA IPHONE NOTES E NOTION



#### Main Feature
- [ ] Check and CheckMate (look at iphone notes)
- [ ] Pawn Promotion
- [ ] Reset
- [ ] MiniMax Player
- [ ] Replay

#### TODO:
- [ ] Leggere regolamento (e implementare regole particolari di mangiare e proteggere re)
- [ ] => Mettere punti cardinali corretti (anche rispetto a neri => NORTH implica andare indietro)
- [ ] RandomPlayer / actually to implement choose and eat opponent's pawns
- [ ] id in ciascuna tile
- [ ] Pawns / MaxNumberSteps => to split by X and by Y
- [ ] ogni tipo di pedina ha una classe per identitificare codice e mosse possibili (base pawn => team, eligible_moves)


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