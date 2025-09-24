# 1. Hash function.
## 1.1 Proprietà.
 - Deterministica.
 - Uniformemente distribuita.
 - Efficiente.
 - Avalanche Effect: if even just one bit changes, then at least half of the 
  output bit change.
## 1.2 Tipi comuni.
 - Division: h(k) = k % H, con H tipicamente primo.
 - Knuth: h(k) = floor(H * (k * A % 1)) dove A è una costante.

 - Universal Hashing  
   L'Universal Hashing serve a proteggersi da attacchi DoS (es.: causare di 
   proposito numerose collisioni e congestionare un bucket) che un attaccante 
   può eseguire dopo aver dedotto quale sia la funzione di Hash usata osservando
   il comportamento di una Hash Table per lunghi periodi.
   Questo Hashing utilizza quindi una Hash function parametrizzabile, così - 
   invece che avere una sola funzione - al variare dei parametri si ha una 
   famiglia di funzioni. Tali parametri sono decisi casualmente e 
   periodicamente, così che un attaccante non abbia mai abbastanza campioni da 
   dedurre l'Hash function. 
   Tale famiglia si definisce k-universal se la probabilità P che k chiavi 
   abbiano lo stesso hash è <= di 1/(H^(k-1)), dove m è la dimensione della Hash
   Table. Questo perchè, la probabilità che k-1 chiavi finiscano nel bucket 
   della chiave 1 è il prodotto delle probabilità 1/H * 1/H ...
   Una famiglia di funzioni comune è il polinomio di grado k-1. Il grado del 
   polinomio dipende da k perché un polinomio di grado k-1 ha k gradi di libertà
   (es. k=2: ax+b, quindi a e b).
   Nella pratica, si usano per lo più famiglie 2-universali, quindi polinomio di
   grado 1: ax+b.
   Tuttavia, la funzione di hash non si ferma al polinomio! La formula completa
   è:  
   ((ax+b)%p)%H, dove p è un numero primo tale che a, b e x sono minori di p.
   Applicare direttamente %H - se MCD(H, a)!=1 - genererebbe un pattern legato a
   H/a che non farebbe distribuire uniformemente le chiavi in tutti i bucket: 
   con a=3 e H=6, le chiavi finirebbero solo in H/a=2 bucket.
   Applicare prima %p permemtte invece di distribuire uniformemente l'ouput del
   polinomio in [0, p-1] grazie alle proprità dei Campi Finiti (o di Galois):
   - un Campo Finito è un insieme Z_p{0, ..., p-1} con le operazioni + e *;
   - le operazioni + e * sone definite come (a+b)%p e (a*b)%p;
   - ha la chiusura, cioè se a e b sono in Z_p anche (a+b)%p e (a*b)%p lo sono;
   - ha l'inverso moltiplicativo, cioè per ogni a esiste A tale che (a*A)%p = 1.
   L'esistenza dell'inverso moltiplicativo garantisce che la moltiplicazione sia
   iniettiva, cioè che ogni input generi un output diverso; e se la 
   moltiplicazione è iniettiva, la distribuzione uniforme dei possiibli valori 
   input [0, p-1] rimane tale invece che ridursi a un sottoinsieme e quindi a 
   usare solo una porzione dei bucket disponibili.
   Con p primo, garantiamo che l'insieme Z_p sia un Campo Finito.

 - Cryptographic Hash  
   A differenza degli altri hash, questi hanno 3 proprietà:
   1. Pre-Image Resistance (o One-Way), se è computazionalmente impossibile 
   risalire dall'hash all'input che l'ha generato;
   2. Second Pre-Image Resistance, se - dato un input - è impossibile trovarne 
   un altro che generi lo stesso hash (2^(H-1) combinazioni se l'hash ha H bit);
   3. Collision Resistance, se è impossibile trovare 2 input che abbiano lo 
   stesso hash (2^(H/2) combinazioni).  
     
   Esempi di Cryptographic Hash:
   - SHA-256
   Gli step di questa funzione sono:
   1. appendere un bit a 1 all'input, quindi il byte 1000'0000 (0x80);
   2. appendi bit a 0 finchè il numero totale di bit S non è S%512==448 (che in
   byte significa %64==56);
   3. appendi la lunghezza originale come intero a 64 bit Big Endian (così 
   l'intero blob diventa lungo un multiplo di 64 byte, 512 bit);
   4. memorizza in K[64] i 32 bit alti della parte frazionaria della radice 
        cubica dei primi 64 numeri primi;
   5. memorizza in H[8] i 32 bit alti della parte frazionaria della radice 
       quadrata dei primi 8 numeri primi;
   6. processa l'intero blob in blocchi da 64 byte:
     - trasforma i 64 byte in un array di 16 words da 4 bytes;
     - queste 16 words diventano i primi 16 valori di un array W di 64 words;
     - le restanti 48 words di W[64] sono combinazioni binarie (quindi non 
       lineari) delle precenti words (Message Schedule step per l'Avalance 
       Effect);
     - copia in {a, ..., h} i valori in {H[0], ..., H[8]};
     - esegui un ciclo di 64 iterazioni dove ad ogni iterazione sovrascrivi i 
        valori in {a, ..., h} con combinazioni di K, W e {a, ..., h};
     - al termine delle 64 iterazioni, somma i valori finali in {a, ..., h} con 
        rispettivamente {H[0], ..., H[8]};
   7. al termine del processamento dell'ultimo blocco di 64 byte, in H[] c'è 
       l'hash SHA-256.
   L'uso delle radici cubiche è dovuto al fatto che le radici quadrate sono già 
   usate nell'inizializzazione dell'algoritmo come eredità del SHA-1.
   L'uso di 32 bit (e non di più) è invece dovuto al fatto che lo standard è 
   nato a inizio 2000 quando ancora i processori a 64 bit non era diffusi.
   L'inizializzazione di K[] è chiaramente fatta una volta staticamente, non ad 
   ogni calcolo dell'hash a runtime.
   Mentre l'inizializzazione di H[] è sia fatta staticamente, sia rinnovata 
   prima di ogni hash visto che i suoi valori vengono sovrascritti.

   - Altri Cryptographic Hash: MD5, SHA-3, BLAKE2.

 - FNV Hash: Veloce per stringhe corte
 - xxHash: Estremamente veloce, ottima per applicazioni real-time
 - Ce ne sono una marea (MurmurHash, djb2, CityHash, ecc.), ma l'xxHash sembra 
    essere il più diffuso tra i non-crittografici.
