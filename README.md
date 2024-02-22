# CHESS

#### TODO:
- [ ] => Mettere punti cardinali corretti (anche rispetto a neri => NORTH implica andare indietro)
- [ ] RandomPlayer / actually to implement choose and eat opponent's pawns
- [ ] GameMode / Check function

- [ ] in random player => toglliere case epawntype@101, gestire tutti i movimenti nelle direzioni cardinali, eat flag dipende da se in newx e newy è NON occupata (occupata da bianco)



- [ ] Pawns / MaxNumberSteps => to split by X and by Y


- [ ] togliere texture dal lato dei cubi

- [ ] aggiornare vettore tile/pedine ad ogni spostamento
- [ ] id in ciascuna tile
- [ ] ogni tipo di pedina ha una classe per identitificare codice e mosse possibili (base pawn => team, eligible_moves)

- [ ] GameMode@69 mettere la classe come parametro della funzione
- [ ] quando clicco su pedina, le tile possibili come mosse cambiano colore
- [ ] Movimento pedine: in classe padre attributo "possible_movements" (definire x e y di arrivo e metodo: "L", "forward", ...)
- [ ] commentare funzioni come ho fatto in php f1 project