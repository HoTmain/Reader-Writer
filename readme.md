<p style="text-align:center; font-size:22px; font-weight:bold">ΑΝΤΡΕΪ-ΑΝΤΡΙΑΝ ΠΡΕΝΤΑ - 1115202000263</p>

<h2 style="text-align:center">Λειτουργικά Συστήματα

1η Εργασία (2022-23)
</h2>

    $ make
    $ ./prnt_p <filename> n r

+ n: γραμμές ανά τμήμα
+ r: αριθμό readers


Το πρόγραμμα είναι μια παραλλαγή του προβλήματος readers-writers, όπου οι readers αλλά και ο writer τρέχουν ταυτόχρονα. Ο συγχρονισμός αυτών έπιτυγχάνεται με semaphores, τα οποία τα περισσότερα από αυτά λειτουργούν ως σήματα (signals), δηλαδή ενημερώνουν ο ένας τον άλλον για το πόσο πρέπει να περιμένουν και πότε να συνεχίσουν στο παρκάτω κομμάτι κώδικα. Στην διαμοιραζόμενη μνήμη υπάρχει ένα struct με μερικές βοηθητικές μεταβλητές, counters και ένας πίνακας όπου θα αποθηκεύεται το τμήμα κειμένου. Ο writer κρατάει στην κοινή μνήμη μόνο το τμήμα που ο reader έχει επιλέξει. Ο reader είναι υπεύθυνος για τη γραμμή που θα διαβάσει.

Αρχικά, ο γονέας πρώτα δημιουργεί όλα τα παιδιά (συγκεκριμένα r συνολικά παιδιά) τα οποία θα παίζουν τον ρόλο των readers 
και, αφού τελειώσει, ξεκινάει και αυτός τον ρόλο του writer, όλα σ’ένα αρχείο. Από όλους τους readers, αυτός που ξεκινάει 
πρώτος έχει το δικαίωμα να διαλέξει ποιό τμήμα θα γραφτεί από τον writer στην κοινή μνήμη. Εδώ χρησιμοποιείται το πρώτο 
(1) signaling, το οποίο απαγορεύει
στον writer να συνεχίσει όσο δεν έχει αποφασιστεί το καινούργιο
τμήμα. Η χρήση του απαγορεύει στον writer να αντικαταστήσει το
τμήμα πρωτού επιλεχτεί ένα νέο από κάποιον reader (αυτό στην πρώτη
επανάληψη ίσως να μην είναι φανερό, αφού ο γονέας μέχρι να φτιάξει
αρχικά όλα τα παιδιά, κάποιο από αυτά θα έχει φτάσει ήδη στο σημείο
όπου επιλέγει κάποιο τμήμα. Αυτό όμως αλλάζει όσο προχωράει η
επανάληψη). Αν αυτό δεν υπήρχε, κάποια τμήματα θα έμπεναν πάνω
από μία φορά στην μνήμη μετά την ολοκλήρωσή τους.

Αφού έχει επιλεχτεί κάποιο τμήμα, οι readers με διαφορετικό μένουν σε εκκρεμότητα. Αυτό επιτυγχάνεται με τη χρήση του semaphore
fcfs. Αυτό στην πραγματικότητα δεν είναι fcfs, αλλά ο scheduler
επιλέγει ποιό θα ξεμπλοκάρει πρώτο, το οποίο είναι αρκετά δίκαιο
στην περίπτωση της εργασίας. Οι readers με ίδιο τμήμα με αυτό που
βρίσκεται στην διαμοιραζόμενη μνήμη περιμένουν πρώτα να γράψει ο
writer το τμήμα στην μνήμη και μετά διαβάζουν τη γραμμή που έχουν
επιλέξει τυχαία. Εδώ βρίσκεται το δεύτερο (2) signaling. Αυτό προφανώς απαγορεύει στους readers να διαβάζουν γραμμές από outdated
τμήμα, δηλαδή τμήμα το οποίο είχε ζητηθεί από προηγούμενο reader.
΄Επειτα διαβάζουν γραμμές επαναληπτικά, μέχρι να επιλέξουν ένα νέο
τμήμα με πιθανότητα 30%.

΄Οταν δεν υπάρχει άλλο ενδιαφέρον για το συγκεκριμένο τμήμα,
όλοι όσοι περιμένουν στον fcfs ξεμπλοκάρονται και αυτός που φτάνει
πρώτος, βέβαια, διαλέγει το τμήμα που θα γραφτεί στην κοινή μνήμη.
Αυτοί με διαφορετικό τμήμα πάλι περιμένουν στον fcfs. Αυτό επαναλαμβάνεται μέχρι να ολοκληρώσουν όλοι οι readers όλες τις 1000 επαναλήψεις.


### Επιπλέον πληροφορίες
- Το τμήμα αποθηκεύεται σ’έναν δισδιάστατο πίνακα char μεγέθους
1024x1024. ΄Ισως να είναι περιοριστικό το σταθερό μέγεθός του, αλλά
για ένα simulation του προβλήματος readers-writers πιστεύω ότι είναι
αρκετός.
- Στο αρχείο καταγραφής, ο χρόνο υποβολής του αιτήματος ξεκινάει
από τότε που ο reader επιλέγει τυχαία κάποιο τμήμα. Ο χρόνος
απάντησης καταγράφεται τη στιγμή που ο reader διαβάζει τη γραμμή
που επιθημεί.
- Αν δοθεί lines per segment μεγαλύτερο από το σύνολο των γραμμών
του αρχείου, εμφανίζει κατάλληλο μήνυμα λάθους.