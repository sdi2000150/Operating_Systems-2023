OperatingSystems-Fall2023-Project3
**************README**************

Όνομα: Θεόδωρος Μωραΐτης
UserID: sdi2000150 (1115202000150)

------------------------------------------------------------------
Πρόγραμμα "readers-writers" με starve-free επιλυση του προβληματος
  (με concurrent processes, semaphores & shared memory segment)   
------------------------------------------------------------------
_______________________________________________________________________________
ΒΑΣΙΚΑ ΣΤΟΙΧΕΙΑ ΤΟΥ ΠΡΟΓΡΑΜΜΑΤΟΣ:

-Χρησιμοποιησα Named POSIX Semaphores, καθως και POSIX Shared Memory Segment.

-Αρχικα, καλουμε το προγραμμα "constructor" για την δημιουργια και αρχικοποιηση των semaphores και του shared memory segment,
 (δινοντας ως ορισμα το ονομα του shared memory segment που θελουμε να δημιουργηθει).

-Ο τροπος εκτελεσης των πολλαπλων readers-writers (προγραμματα "reader" και "writer") γινεται εκτελωντας τους απο διαφορετικα terminals,
 (δινοντας ως ορισμα και το ονομα του shared memory segment που δημιουργησαμε με τον constructor).

-Η λυση ειναι ελευθερα-λιμου, τοσο για τους readers οσο και για τους writers.

-Τα τελικα στατιστικα υπολογιζονται και ενημερωνονται κατα την εκτελεση των reader-writers, ωστοσο η εμφανιση τους
 γινεται αφου ολοκληρωθουν ολοι οι readers-writers που θελουμε, και καλωντας manually το εκτελεσιμο "results",
 (δινοντας ως ορισμα το ονομα του shared memory segment που δημιουργησαμε με τον constructor).

-Τελος, καλουμε το προγραμμα "destructor" για την διαγραφη των semaphores και του shared memory segment που δημιουργηθηκαν και χρησιμοποιηθηκαν,
 (δινοντας ως ορισμα το ονομα του shared memory segment που ειχαμε δημιουργησει με τον constructor).

_______________________________________________________________________________
COMPILATION/EXECUTION ΤΟΥ ΠΡΟΓΡΑΜΜΑΤΟΣ:

1) Separate compilation του προγράμματος, με χρήση Makefile, στο tty μέσω της εντολής:
 > make
 ,με την οποία δημιουργουνται τα εκτελέσιμα "constructor", "reader", "writer", "results" και "destructor".

2) Εκτέλεση του προγράμματος, εκτελωντας τα επιμερους προγραμματα απο διαφορετικα ttys ως εξης:

 2.1) Αρχικα, εκτελουμε τον constructor ως εξης:
  > ./constructor -s shmname

 2.2) Επειτα, εκτελουμε (απο διαφορετικα και πολλαπλα terminals) τους readers-writers (οσες φορες και με οποια σειρα θελουμε), ως εξης:
  > ./reader -f filename -l recid1[ recid2] -d time -s shmname
  > ./writer -f filename -l recid -v value -d time -s shmname

 2.3) Αφου εκτελεσουμε οσους readers-writers θελουμε, και ολοκληρωθουν, καλουμε την results ωστε να παρουμε το αποτελεσμα των στατιστικων:
  > ./results -s shmname

 2.4) Τελος, καλουμε τον destructor ως εξης:
  > ./destructor -s shmname

3) Clean των .ο και των εκτελέσιμων που δημιουργήθηκαν μετά το compilation:
 > make clean

4) Για γρηγορη εμφανιση των ενεργων semaphores και shmsegments στο συστημα (αντι για την εντολη ls /dev/shm):
 > make ls

Σχόλια για τα ορισματα κατα την κληση των εκτελεσιμων:

-Το shmname ειναι το ονομα (καποιο string) του shared memory segment που θα δημιουργηθει (απο τον constructor), 
 θα χρησιμοποιηθει (απο readers-writers και την results) και εν τελη θα διαγραφει (απο τον destructor).
-Το filename είναι ένα αρχείο που περιέχει πολλαπλά Records σε binary μορφή (ενα εκ των accounts50.bin, accounts5000.bin, 
 accounts50000.bin, accounts100000.bin). 
-Το recid (& recid1, recid2) είναι ακέραιος >= 1 (και <= num_of_accounts σε σχεση με το ποιο αρχειο δινεται).
-Το time ειναι ενας ακεραιος >= 1.
-Το value ειναι ενας ακεραιος (ειτε αρνητικος ειτε θετικος ειτε μηδεν).

-Τα "σύνολα" των arguments "-s shname", "-f filename", "-l recid" (ή "-l recid1[ recid2]"), "-d time", "-v value"
 μπορούν να δοθούν μεταξύ τους σε τυχαία σειρά (με το εκάστοτε flag + argument να είναι μαζί και με αυτή την σειρά).

-Στην περιπτωση της κλησης του reader με δυο recids, τα ορισματα recid1 recid2 χωριζονται μεταξυ τους με κενο/κενα.

_______________________________________________________________________________
ΑΛΓΟΡΙΘΜΟΣ ΠΟΥ ΧΡΗΣΙΜΟΠΟΙΗΘΗΚΕ:

/* Solution to the Readers-Writers problem, with no starvation issue to either readers or writers */

/* PSEUDOCODE of the Algorithm used */  //a variant of the one we discussed in class, 
                                        //but with the addition of a semaphore use (in), 
                                        //in order to solve the starvation of writer issue

/* SHARED MEMORY:
    semaphore mutex->value = 1; //for mutual exclusion when readers access the Critical Section
    semaphore wrt->value = 1;   //for sychronization among writers (mutual exclusion) and among readers-writers
    semaphore in->value = 1;    //for ensuring the starve-free access for both readers and writers
    int readcount = 0;          //number of active readers reading in file concurrently
*/

/* READER:
    P(in);                      //wait in line to be serviced
    P(mutex);                   //for CS of reader
    readcount++;
    if (readcount == 1) P(wrt); //when first reader, disable any forthcoming writer /or when writer active, suspend 1st reader
    V(mutex);                   //for CS of reader
    V(in);                      //let next in line be serviced
    ...
    Reading in progress...      //many readers concurrently
    ...
    P(mutex);                   //for CS of reader
    readcount--;
    if (readcount == 0) V(wrt); //when no readers, enable any forthcoming writer /or let readers rejoin
    V(mutex);                   //for CS of reader
*/

/* WRITER:
    P(in);                      //wait in line to be serviced
    P(wrt);                     //suspend 1st writer /or suspend 1st (and rest) reader(s) /or suspend rest writers
    ...
    Writing in progress...      //one writer at a time
    ...
    V(wrt);                     //enable any suspended reader /or enalbe one suspended writer
    V(in);                      //let next in line be serviced
*/

_______________________________________________________________________________
SOURCE/HEADER FILES ΤΟΥ ΠΡΟΓΡΑΜΜΑΤΟΣ:

structs.h: Παρεχει την δηλωση της δομης "shm_seg" (η οποια χρησιμοποιειται ως το shared memory segment),
           και της δομης "Account" (η οποια αποτελει την μορφη μιας εγγραφης εντος του δυαδικου αρχειου).

constructor.c: Δημιουργει και αρχικοποιει του σημαφορους (named posix semaphores),
               καθως και το κοινο κομματι μνημης (posix shared memory segment).
               Οι semaphores ονομαζονται ως "sem_mutex", "sem_wrt", "sem_in" (hard-coded απο το προγραμμα),
               ενω το shared memory segment ονομαζεται με το ονομα που παρεχει ο χρηστης κατα την κληση του constructor.

reader.c: Αποτελει τον "Reader". Περιεχει ολη την υλοποιηση του αλγοριθμου του, και κανει ολες τις ζητουμενες ενεργειες.
          Εφοσον εχει κληθει ο constructor, μπορει να κληθει πολλαπλες φορες (ταυτοχρονα απο πολλαπλα terminals,
          ειτε και ταυτοχρονα με πολλαπλους writers).

writer.c: Αποτελει τον "Writer". Περιεχει ολη την υλοποιηση του αλγοριθμου του, και κανει ολες τις ζητουμενες ενεργειες.
          Εφοσον εχει κληθει ο constructor, μπορει να κληθει πολλαπλες φορες (ταυτοχρονα απο πολλαπλα terminals,
          ειτε και ταυτοχρονα με πολλαπλους readers).

results.c: Τυπωνει τα στατιστικα στοιχεια που βρισκονται εντος του shared memory segment, και που εχουν δημιουργηθει
           απο την μεχρι-τωρα εκτελεση των readers-writers.

destructor.c: Διαγραφει απο το συστημα τους semaphores και το shared memory segment που δημιουργηθηκαν απο τον constructor.

_______________________________________________________________________________
ΣΧΕΔΙΑΣΤΙΚΕΣ ΕΠΙΛΟΓΕΣ ΤΟΥ ΠΡΟΓΡΑΜΜΑΤΟΣ:

-Υλοποιησα τον παραπανω αλγοριθμο για την επιλυση του προβληματος χωρις starvation. Ουσιαστικα ειναι παρομοιος με αυτον που
 εχουμε δει στο μαθημα, με μια επιπλεον προσθηκη, τον σημαφορο "in". Αυτος ο εξτρα σημαφορος κραταει την προτεραιοτητα με την
 οποια θα εισελθει ο καθενας (ειτε reader ειτε writer), με βαση την στιγμη αφιξης του. 
 Για παραδειγμα, αν readers διαβαζουν στο αρχειο (δηλαδη βρισκονται εντος του reading section) και προσπαθησει να εισελθει καποιος writer, 
 τοτε ο writer θα κατεβασει τον σημαφορο "in", αρα θα παρει προτεραιοτητα (και θα περιμενει πανω στον "wrt" μεχρι να τελειωσουν 
 οι υπαρχοντες readers). Ετσι, καποιος επιπλεον reader που εκεινη τη στιγμη θα προσπαθησει να μπει, δεν θα τα καταφερει, 
 και θα περιμενει πανω στον "in". 
 Αυτο και αλλα σεναρια/παραδειγματα δειχνουν οτι αυτη η λυση ειναι σωστη και δεν προκαλει λιμο σε κανεναν (ειτε reader ειτε writer).

-Το flag/ορισμα -s shmid, ουσιαστικα ειναι -s shmname (αφου χρησιμοποιηθηκε posix shared memory segment). Αυτο χρειαζεται ως ορισμα
 σε ολα τα προγραμματα ("constructor", "reader", "writer", "results", "destructor"). Στον constructor ειναι επιλογη του χρηστη το 
 ποιο θα ειναι το ονομα, και στα υπολοιπα εκτελεσιμα θα πρεπει να δωθει το ιδιο ονομα που δωθηκε και στον constructor.

-Στην περιπτωση κλησης του reader, τα ορισματα των recids (οταν εχουμε δυο, δηλαδη συνολο απο records) δεν ειναι -l recid,recid 
 αλλά -l recid recid (δηλαδη με κενο/κενα αναμεσα στα δυο recids).

-Χρησιμοποιηθε ενας τροπος (η συναρτηση "extractInteger") για να βρεθει το πληθος των εγγραφων του αρχειου μεσα απο το ονομα του αρχειου,
 γι' αυτο και το ονομα του δυαδικου αρχειου θα πρεπει να ειναι της μορφης "accountsX.bin", δηλαδη οπως και τα αρχεια τεστινγκ που δωθηκαν
 (με Χ ακεραιος, πχ 50, 5000, κλπ).

-Οι πινακες των ενεργων Readers και των ενεργων Writers (εντος shared memory segment), εχουν fixed μεγεθος (upper limit).
 Στην ακραια περιπτωση που εισελθουν περισσοτεροι ενεργοι readers ή ενεργοι writers απο το limit, τοτε γινεται true ενα flag
 (extra_Readers και extra_Writers αντιστοιχα), που δειχνει οτι ξεπεραστηκε το οριο των max active readers-writers αντιστοιχα.

-2o στατιστικο: μετρησα τον χρονο που περασε απο την αρχη της main μεχρι και την ολοκληρωση των λειτουργιων του καθε reader,
 δηλαδη entry section + 1st critical section + reading section + 2nd critical section.

-4o στατιστικο: μετρησα τον χρονο που περασε απο την αρχη της main μεχρι και την ολοκληρωση των λειτουργιων του καθε writer,
 δηλαδη entry section + critical section/writing section.

-5o στατιστικο: μετρησα τον μεγιστο χρονο που περασε απο την αρχη της main μεχρι και το ξεκινημα της βασικης δουλειας
 ειτε reader (reading section) ειτε writer (writing section).

-6ο στατιστικο: κατεγραψα το πληθος των εγγραφων που ειτε διαβαστηκαν ειτε γραφτηκαν απο readers/writers, ως εξης:
 αν, πχ, η 5η εγγραφη διαβαστηκε/γραφτηκε 13 φορες, τοτε προσμετραται ως 13 στο πληθος αυτο.

_______________________________________________________________________________
ΠΑΡΑΔΟΧΕΣ ΤΟΥ ΠΡΟΓΡΑΜΜΑΤΟΣ:

-Το προγραμμα "results", που τυπωνει τα στατιστικα, θα πρεπει να κληθει εφοσον εχουν τερματισει ολοι οι readers-writers
 οι οποιοι εχουν κληθει μεχρι εκεινη την στιγμη, ετσι ωστε να μην δημιουργηθει προβλημα με oudated statistics (και αυτο γιατι 
 αυτο το προγραμμα δεν συμμετεχει σε καποιο συγχρονισμο με readers-writers, δηλαδη δεν χρησιμοποιει semaphores για να κανει
 access (read-only) το shared memory segment).

_______________________________________________________________________________
ΕΛΛΕΙΨΕΙΣ ΤΟΥ ΠΡΟΓΡΑΜΜΑΤΟΣ:

-Δεν υλοποιησα την περιπτωση καποιος writer να γραφει μια εγγραφη, και την ιδια στιγμη να μπορουν readers να διαβαζουν
 διαφορετικες εγγραφες ή να μπορουν writers να γραφουν διαφορετικες εγγραφες.
 Δηλαδη, στην υλοποιηση μου, οταν ενας writer αρχιζει να δουλευει με μια εγγραφη, κανενας αλλος δεν γινεται δεκτος.