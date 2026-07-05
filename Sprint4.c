#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#pragma warning (disable:4996)

enum {
    MAX_ETUDIANTS = 100,
    MAX_NOM = 31,
    NB_UE = 6,
    NB_SEMESTRES = 6,
    NOTE_INCONNUE = -1,
    UE_MIN = 1,
    LONGUEUR_MAX = 256,
    NB_BILANS = 3,
    NB_ANNEES = 3
};

const float NOTE_MAX = 20.f, NOTE_MIN = 0.f, NOTE_ADMISSION = 10.f;

typedef enum {
    INCONNU, ADM, ADC, ADS, AJ, AJB
} CodeValidation;

typedef enum {
    EN_COURS, DEMISSION, DEFAILLANCE, AJOURNE, DIPLOME, TERMINE
} Etat;

typedef struct {
    float note;
    CodeValidation code;
} NoteUE;

typedef struct {
    int id;
    char prenom[MAX_NOM];
    char nom[MAX_NOM];
    int semestre_actuel;
    Etat etat;
    NoteUE notes[NB_SEMESTRES][NB_UE];
    NoteUE bilans[NB_BILANS][NB_UE];
    Etat bilan;
} Etudiant;

/* Maquettes */
void InitialiserEtudiant(Etudiant* e, int id, const char* prenom, const char* nom);
int ChercherEtudiant(const char* prenom, const char* nom, int nb_etudiants, Etudiant* etudiants);
void InscrireEtudiant(const char* prenom, const char* nom, int* nb_etudiants, Etudiant* etudiants);
void AfficheEtudiants(const Etudiant etudiants[], int nb_etudiants);
void AjouterNote(Etudiant* etudiants, int nb_etudiants, int id, int num_ue, float note);
void AjouterNoteUE(Etudiant* e, int num_ue, float note);
const char* CodeString(CodeValidation code);
const char* EtatString(Etat etat);
void AfficherNote(float note);
void CursusEtudiant(int id, int nb_etudiants, Etudiant* etudiants);
int verifid(int id, int nb_etudiants, Etudiant* etudiants);
void DemissionOuDefaillance(int id, const char commande[], int nb_etudiants, Etudiant* etudiants);
int ToutesNotesPresentes(Etudiant* e, int semestre);
void JurySemestre(int num_semestre, int nb_etudiants, Etudiant* etudiants);
void CompensationAnnee(Etudiant* etudiants, int semestre_idx, int ue, int i);
void CompensationAnneePrecedente(Etudiant* etudiants, int semestre_idx, int ue, int i);
void BilanAnnee1(Etudiant* etudiants, int semestre_idx, int i, int* nb_etudiants_passes, int num_semestre, int* nb_ue_valide, int* nb_ue_aj);
void BilanAnnee2(Etudiant* etudiants, int semestre_idx, int i, int* nb_etudiants_passes, int num_semestre, int* nb_ue_valide, int* nb_ue_aj, int* nb_ue_ajb);
void BilanAnnee3(Etudiant* etudiants, int semestre_idx, int i, int* nb_etudiants_passes, int num_semestre, int* nb_ue_valide);
void BilanFinal(int num_annee, int nb_etudiants, Etudiant* etudiants);

int main(void) {
    char commande[LONGUEUR_MAX];
    char prenom[MAX_NOM];
    char nom[MAX_NOM];
    int id, num_ue, num_semestre, num_annee;
    float note;
    int nb_etudiants = 0; /* Nombre d'étudiants inscrits */
    Etudiant etudiants[MAX_ETUDIANTS]; /* Tableau des étudiants */

    while (1) {
        scanf("%s", commande);
        if (strcmp(commande, "EXIT") == 0) {
            return 0;
        }
        else if (strcmp(commande, "INSCRIRE") == 0) {
            scanf("%s %s", prenom, nom);
            InscrireEtudiant(prenom, nom, &nb_etudiants, etudiants);
        }
        else if (strcmp(commande, "CURSUS") == 0) {
            scanf("%d", &id);
            CursusEtudiant(id, nb_etudiants, etudiants);
        }
        else if (strcmp(commande, "ETUDIANTS") == 0) {
            AfficheEtudiants(etudiants, nb_etudiants);
        }
        else if (strcmp(commande, "NOTE") == 0) {
            scanf("%d %d %f", &id, &num_ue, &note);
            AjouterNote(etudiants, nb_etudiants, id, num_ue, note);
        }
        else if (strcmp(commande, "DEMISSION") == 0 || strcmp(commande, "DEFAILLANCE") == 0) {
            scanf("%d", &id);
            DemissionOuDefaillance(id, commande, nb_etudiants, etudiants);
        }
        else if (strcmp(commande, "JURY") == 0) {
            scanf("%d", &num_semestre);
            JurySemestre(num_semestre, nb_etudiants, etudiants);
        }
        else if (strcmp(commande, "BILAN") == 0) {
            scanf("%d", &num_annee);
            BilanFinal(num_annee, nb_etudiants, etudiants);
        }
    }
    return 0;
}

/* Fonction correspondant a la commande INSCRIRE */
void InitialiserEtudiant(Etudiant* e, int id, const char* prenom, const char* nom) {
    assert(e != NULL);
    assert(id > 0);
    assert(prenom != NULL);
    assert(nom != NULL);

    e->id = id;
    strcpy(e->prenom, prenom);
    strcpy(e->nom, nom);
    e->semestre_actuel = 1;  /* Premier semestre par défaut */
    e->etat = EN_COURS;
    e->bilan = EN_COURS;
    /* Initialiser toutes les notes comme inconnues */
    for (int s = 0; s < NB_SEMESTRES; s++) {
        for (int ue = 0; ue < NB_UE; ue++) {
            e->notes[s][ue].note = NOTE_INCONNUE;
            e->notes[s][ue].code = INCONNU;
        }
    }
    for (int b = 0; b < NB_BILANS; b++) {
        for (int ue = 0; ue < NB_UE; ue++) {
            e->bilans[b][ue].note = NOTE_INCONNUE;
            e->bilans[b][ue].code = INCONNU;
        }
    }
}

int ChercherEtudiant(const char* prenom, const char* nom, int nb_etudiants, Etudiant* etudiants) {
    assert(prenom != NULL);
    assert(nom != NULL);

    for (int i = 0; i < nb_etudiants; i++) {
        if (strcmp(etudiants[i].prenom, prenom) == 0 &&
            strcmp(etudiants[i].nom, nom) == 0) {
            return i;
        }
    }
    return -1;
}

void InscrireEtudiant(const char* prenom, const char* nom, int* nb_etudiants, Etudiant* etudiants) {
    assert(prenom != NULL);
    assert(nom != NULL);

    /* Vérifier si un étudiant avec le même nom existe déjà */
    if (ChercherEtudiant(prenom, nom, *nb_etudiants, etudiants) != -1) {
        printf("Nom incorrect\n");
        return;
    }

    /* Vérifier qu'on ne dépasse pas le maximum d'étudiants */
    if (*nb_etudiants >= MAX_ETUDIANTS) {
        printf("Erreur: nombre maximum d'etudiants atteint\n");
        return;
    }

    /* Créer le nouvel étudiant */
    (*nb_etudiants)++;
    InitialiserEtudiant(&etudiants[*nb_etudiants - 1], *nb_etudiants, prenom, nom);

    printf("Inscription enregistree (%d)\n", *nb_etudiants);
}

/* Fonction permettant de vérifier l'exactitudes des informations entrées */
void AjouterNote(Etudiant* etudiants, int nb_etudiants, int id, int num_ue, float note) {
    int SemestreEnCours = -1;
    if (num_ue > NB_UE || num_ue < UE_MIN) {
        printf("UE incorrect\n");
        return;
    }
    if (note > NOTE_MAX || note < NOTE_MIN) {
        printf("Note incorrecte\n");
        return;
    }
    SemestreEnCours = verifid(id, nb_etudiants, etudiants);
    if (SemestreEnCours != -1) {
        AjouterNoteUE(&etudiants[SemestreEnCours], num_ue, note);
        printf("Note enregistree\n");
    }
}
/* Fonction qui enregistre une nouvelle note à l'ue choisie du dernier semestre et son code*/
void AjouterNoteUE(Etudiant* e, int num_ue, float note) {
    int semestre = e->semestre_actuel - 1;
    e->notes[semestre][num_ue - 1].note = note;
    e->notes[semestre][num_ue - 1].code = (note >= NOTE_ADMISSION) ? ADM : AJ;
}
/* Vérifie que l'étudiant choisi rentre dans les critères (id correct, étudiant avec semestre en cours) */
int verifid(int id, int nb_etudiants, Etudiant* etudiants) {
    int SemestreEnCours = -1;
    if (id > nb_etudiants || id < 1) {
        printf("Identifiant incorrect\n");
        return -1;
    }

    for (int i = 0; i < nb_etudiants; ++i) {
        if (etudiants[i].id == id && etudiants[i].etat == EN_COURS) {
            SemestreEnCours = i;
            break;
        }
    }

    if (SemestreEnCours < 0) {
        printf("Etudiant hors formation\n ");
        return -1;
    }
    return SemestreEnCours;
}

/* Fonction annexe à CURSUS, convertis les codes des notes en caractères afficheables (strings) */
const char* CodeString(CodeValidation code) {
    switch (code) {
    case INCONNU: return "*";
    case ADM: return "ADM";
    case AJ: return "AJ";
    case ADC: return "ADC";
    case ADS: return "ADS";
    case AJB: return "AJB";
    default: return "?";
    }
}
/* Fonction annexe à CURSUS, convertis les états des semestres et bilans en caractères afficheables (strings) */
const char* EtatString(Etat etat) {
    switch (etat) {
    case EN_COURS: return "en cours";
    case DEMISSION: return "demission";
    case DEFAILLANCE: return "defaillance";
    case AJOURNE: return "ajourne";
    case DIPLOME: return "diplome";
    case TERMINE: return "";
    default: return "inconnu";
    }
}
/* Affiche à l'écran la note des UEs selon si elle est connue ou non */
void AfficherNote(float note) {
    if (note == NOTE_INCONNUE) {
        printf("*");
    }
    else {
        /* Tronquer à une décimale sans arrondir */
        printf("%.1f", floorf(note * 10.f) / 10.f);
    }
}
/* Affiche toutes les notes de l'étudiant choisi (tous les semestres et bilans) */
void CursusEtudiant(int id, int nb_etudiants, Etudiant* etudiants) {
    /* Vérifier que l'id est valide */
    if (id < 1 || id > nb_etudiants) {
        printf("Identifiant incorrect\n");
        return;
    }
    Etudiant* e = &etudiants[id - 1];
    printf("%d %s %s\n", e->id, e->prenom, e->nom);

    /* Affiche chaque semestre suivi et le bilan de chaque année */
    for (int s = 0; s < e->semestre_actuel; s++) {
        printf("S%d - ", s + 1);
        for (int ue = 0; ue < NB_UE; ue++) { /* Pour chaque semestre, affiche les notes des UEs */
            AfficherNote(e->notes[s][ue].note);
            printf(" (%s) - ", CodeString(e->notes[s][ue].code));
        }
        /* Affiche l'etat */
        if (s == e->semestre_actuel - 1 && e->bilan != AJOURNE) {
            printf("%s\n", EtatString(e->etat));
        }
        printf("\n");
        /* Afficher le bilan 1 après le semestre 2 si celui-ci est bien connu */
        if (s == 1 && e->bilans[0][0].note != NOTE_INCONNUE) {
            printf("B1 - ");
            for (int ue = 0; ue < NB_UE; ++ue) {
                AfficherNote(e->bilans[0][ue].note);
                printf(" (%s) - ", CodeString(e->bilans[0][ue].code));
            }
            if (e->bilans[1][0].note == NOTE_INCONNUE) /* Si le bilan de l'année suivante existe, alors pas d'état à afficher (diplômé, ajourné...) */
                printf("%s\n", EtatString(e->bilan));
            else
                printf("\n");
        }
        /* Afficher le bilan 2 après le semestre 4 si celui-ci est bien connu */
        else if (s == 3 && e->bilans[1][0].note != NOTE_INCONNUE) {
            printf("B2 - ");
            for (int ue = 0; ue < NB_UE; ++ue) {
                AfficherNote(e->bilans[1][ue].note);
                printf(" (%s) - ", CodeString(e->bilans[1][ue].code));
            }
            if (e->bilans[2][0].note == NOTE_INCONNUE) /* Si le bilan de l'année suivante existe, alors pas d'état à afficher (diplômé, ajourné...) */
                printf("%s\n", EtatString(e->bilan));
            else
                printf("\n");
        }
        /* Afficher le bilan 3 après le semestre 6 si celui-ci est bien connu */
        if (s == 5 && e->bilans[2][0].note != NOTE_INCONNUE) {
            printf("B3 - ");
            for (int ue = 0; ue < NB_UE; ++ue) {
                AfficherNote(e->bilans[2][ue].note);
                printf(" (%s) - ", CodeString(e->bilans[2][ue].code));
            }
            printf("%s\n", EtatString(e->bilan));
        }
    }
}


/* Fonction correspondant à la commande ETUDIANTS, affiche tous les étudiants enregistrés et leurs infos */
void AfficheEtudiants(const Etudiant etudiants[], int nb_etudiants) {
    for (int i = 0; i < nb_etudiants; ++i) {
        printf("%d - %s %s - S%d - %s\n", etudiants[i].id, etudiants[i].prenom, etudiants[i].nom, etudiants[i].semestre_actuel, (etudiants[i].bilan==EN_COURS || etudiants[i].bilan==TERMINE)? EtatString(etudiants[i].etat): EtatString(etudiants[i].bilan));
    }
}

/* Fonction correspondant aux commandes defaillance ou demission */
void DemissionOuDefaillance(int id, const char commande[], int nb_etudiants, Etudiant* etudiants) {
    int SemestreEnCours = verifid(id, nb_etudiants, etudiants);
    if (SemestreEnCours > -1) {
        Etudiant* e = &etudiants[SemestreEnCours];
        if (strcmp(commande, "DEMISSION") == 0) {
            e->etat = DEMISSION;
            printf("Demission enregistree\n");
        }
        if (strcmp(commande, "DEFAILLANCE") == 0) {
            e->etat = DEFAILLANCE;
            printf("Defaillance enregistree\n");
        }
    }
}

/* Fonction correspondant a la commande JURY (semestre impair) */
int ToutesNotesPresentes(Etudiant* e, int semestre) {
    assert(e != NULL);
    assert(semestre >= 0 && semestre < NB_SEMESTRES);

    for (int ue = 0; ue < NB_UE; ue++) {
        if (e->notes[semestre][ue].note == NOTE_INCONNUE) {
            return 0;
        }
    }
    return 1;
}

/* Fonction permettant le jury des semestres pairs et impairs */
void JurySemestre(int num_semestre, int nb_etudiants, Etudiant* etudiants) {
    /* Vérifier que le numéro de semestre est valide */
    if (num_semestre < 1 || num_semestre > NB_SEMESTRES) {
        printf("Semestre incorrect\n");
        return;
    }

    int semestre_idx = num_semestre - 1;
    int nb_etudiants_passes = 0;

    /* Faire passer les étudiants au semestre suivant en vérifiant les conditions requise de passage au semestre supérieur */
    for (int i = 0; i < nb_etudiants; i++) { /* Pour chaque étudiant */
        Etudiant* e = &etudiants[i];
        int nb_ue_aj = 0;
        int nb_ue_ajb = 0;
        int nb_ue_valide = 0;

        /* Vérifier qu'aucune note n'est manquante pour les étudiants concernés du semestre en cours */
        if (e->etat == EN_COURS && e->semestre_actuel == num_semestre) {
            if (!ToutesNotesPresentes(e, semestre_idx)) {
                printf("Des notes sont manquantes\n");
                return;
            }
        }

        if (e->etat == EN_COURS && e->semestre_actuel == num_semestre && num_semestre % 2 != 0) {
            /* En semestre impair, passage automatique au semestre suivant */
            e->semestre_actuel++;
            nb_etudiants_passes++;
        }
        else if ((e->etat == EN_COURS && e->semestre_actuel == num_semestre && num_semestre == 2)) /* En semestre pair, calcul des moyennes pour le bilan d'année 1 (fin semestre 2) */
            BilanAnnee1(etudiants, semestre_idx, i, &nb_etudiants_passes, num_semestre, &nb_ue_valide, &nb_ue_aj);
        else if (e->etat == EN_COURS && e->semestre_actuel == num_semestre && num_semestre == 4) /* En semestre pair, calcul des moyennes pour le bilan d'année 2 (fin semestre 4) */
            BilanAnnee2(etudiants, semestre_idx, i, &nb_etudiants_passes, num_semestre, &nb_ue_valide, &nb_ue_aj, &nb_ue_ajb);
        else if ((e->etat == EN_COURS && e->semestre_actuel == num_semestre && num_semestre == 6)) /* En semestre pair, calcul des moyennes pour le bilan d'année 3 (fin semestre 6) */
            BilanAnnee3(etudiants, semestre_idx, i, &nb_etudiants_passes, num_semestre, &nb_ue_valide);
    }
    printf("Semestre termine pour %d etudiant(s)\n", nb_etudiants_passes);

}
/* Si la moyenne de l'année est suffisante dans une ue, le semestre de cet année dont cet ue n'est pas validé est compensé */
void CompensationAnnee(Etudiant* etudiants, int semestre_idx, int ue, int i) {
    Etudiant* e = &etudiants[i];
    if (e->notes[semestre_idx - 1][ue].code == AJ) /* Si le premier semestre de l'année n'est pas validé, mettre le code ADC */
        e->notes[semestre_idx - 1][ue].code = ADC;
    if (e->notes[semestre_idx][ue].code == AJ) /* Si le deuxième semestre de l'année n'est pas validé, mettre le code ADC */
        e->notes[semestre_idx][ue].code = ADC;
}
/* Si la moyenne de l'année est suffisante dans une ue, le(s) semestre(s) de l'année précédente dont cet ue n'est pas validé est compensé */
void CompensationAnneePrecedente(Etudiant* etudiants, int semestre_idx, int ue, int i) {
    Etudiant* e = &etudiants[i];
    if (e->notes[semestre_idx - 2][ue].code == AJ) { /* Si le premier semestre de l'année précédente n'est pas validé, mettre le code ADS (ainsi qu'au bilan) */
        e->notes[semestre_idx - 2][ue].code = ADS;
        e->bilans[(semestre_idx-1)/2 - 1][ue].code = ADS;
    }
    if (e->notes[semestre_idx - 3][ue].code == AJ) { /* Si le deuxième semestre de l'année précédente n'est pas validé, mettre le code ADS (ainsi qu'au bilan) */
        e->notes[semestre_idx - 3][ue].code = ADS;
        e->bilans[(semestre_idx-1)/2 - 1][ue].code = ADS;
    }
}

void BilanAnnee1(Etudiant* etudiants, int semestre_idx, int i, int* nb_etudiants_passes, int num_semestre, int* nb_ue_valide, int* nb_ue_aj) {
    Etudiant* e = &etudiants[i];
    for (int ue = 0; ue < NB_UE; ++ue) { /* Pour chaque ue on fait la moyenne des deux semestres */
        float moyenne = (e->notes[0][ue].note + e->notes[1][ue].note) / 2.0f;
        e->bilans[0][ue].note = moyenne;
        if (moyenne >= 10) {
            e->bilans[0][ue].code = ADM;
            ++(*nb_ue_valide);
            CompensationAnnee(etudiants, semestre_idx, ue, i);
        }
        else if (moyenne < 10 && moyenne >= 8)
            e->bilans[0][ue].code = AJ;
        else if (moyenne < 8) {
            e->bilans[0][ue].code = AJB;
            ++(*nb_ue_aj);
        }
    }
    (*nb_etudiants_passes)++;
    if (*nb_ue_aj == 0 && *nb_ue_valide > 3) { /* Si l'année est validée, le bilan est terminé et on peu passé au semestre supérieur, qui sera en cours */
        e->semestre_actuel++;
        e->bilan = TERMINE;
        e->etat = EN_COURS;
    }
    else { /* Sinon le semestre en cours est terminé et ajourné dans le bilan. Cela met fin au cursus de l'étudiant */
        e->bilan = AJOURNE;
        e->etat = TERMINE;
    }
}

void BilanAnnee2(Etudiant* etudiants, int semestre_idx, int i, int* nb_etudiants_passes, int num_semestre, int* nb_ue_valide, int* nb_ue_aj, int* nb_ue_ajb) {
    Etudiant* e = &etudiants[i];
    for (int ue = 0; ue < NB_UE; ++ue) {
        float moyenne = (e->notes[2][ue].note + e->notes[3][ue].note) / 2.0f;
        e->bilans[1][ue].note = moyenne;
        if (moyenne >= 10) {
            e->bilans[1][ue].code = ADM;
            ++(*nb_ue_valide);
            CompensationAnnee(etudiants, semestre_idx, ue, i);
            CompensationAnneePrecedente(etudiants, semestre_idx, ue, i);
        }
        else if (moyenne < 10 && moyenne >= 8) {
            e->bilans[1][ue].code = AJ;
        }
        else if (moyenne < 8) {
            e->bilans[1][ue].code = AJB;
            ++(*nb_ue_ajb);
        }
    }
    int nb_ue_valide_an1 = 0;
    for (int ue = 0; ue < NB_UE; ++ue) { /* Vérification du nombre d'ue validés l'année précédente */
        if (e->bilans[0][ue].code != AJB && e->bilans[0][ue].code != AJ)
            ++nb_ue_valide_an1;
    }
    (*nb_etudiants_passes)++;
    if (*nb_ue_ajb == 0 && *nb_ue_valide > 3 && nb_ue_valide_an1 == 6) {/* Si l'année esr validée, le bilan est terminé et on peu passé au semestre supérieur, qui sera en cours */
        e->semestre_actuel++;
        e->bilan = TERMINE;
        e->etat = EN_COURS;
    }
    else { /* Sinon le semestre en cours est terminé et ajourné dans le bilan. Cela met fin au cursus de l'étudiant */
        e->etat = TERMINE;
        e->bilan = AJOURNE;
    }
}

void BilanAnnee3(Etudiant* etudiants, int semestre_idx, int i, int* nb_etudiants_passes, int num_semestre, int* nb_ue_valide) {
    Etudiant* e = &etudiants[i];
    for (int ue = 0; ue < NB_UE; ++ue) {
        float moyenne = (e->notes[4][ue].note + e->notes[5][ue].note) / 2.0f;
        e->bilans[2][ue].note = moyenne;
        if (moyenne >= 10) {
            e->bilans[2][ue].code = ADM;
            CompensationAnnee(etudiants, semestre_idx, ue, i);
            CompensationAnneePrecedente(etudiants, semestre_idx, ue, i);
        }
        else if (moyenne < 10 && moyenne >= 8)
            e->bilans[2][ue].code = AJ;
        else if (moyenne < 8) {
            e->bilans[2][ue].code = AJB;
        }
    }
    for (int sem = 0; sem < NB_SEMESTRES; ++sem) {
        for (int ue = 0; ue < NB_UE; ++ue) {
            if (e->notes[sem][ue].code != AJ)
                ++(*nb_ue_valide);
        }
    }
    (*nb_etudiants_passes)++;
    if (*nb_ue_valide == 36) { /* Si l'année est validée, le semestr en cours et terminé et il est diplômé */
        e->etat = TERMINE;
        e->bilan = DIPLOME;
    }
    else { /* Sinon le semestre en cours est terminé et ajourné dans le bilan. Cela met fin au cursus de l'étudiant */
        e->etat = TERMINE;
        e->bilan = AJOURNE;
    }
}

void BilanFinal(int num_annee, int nb_etudiants, Etudiant* etudiants) {
    /* Vérifier que l'année est valide */
    if (num_annee < 1 || num_annee > NB_ANNEES) {
        printf("Annee incorrecte\n");
        return;
    }

    /* Calculer le premier semestre de l'année : Année 1 → S1 (1); Année 2 → S3 (3); Année 3 → S5 (5) */
    int semestre_min = (num_annee - 1) * 2 + 1;

    /* Initialiser les compteurs */
    int nb_demission = 0;
    int nb_defaillance = 0;
    int nb_en_cours = 0;
    int nb_ajourne = 0;
    int nb_passe = 0;

    /* Parcourir tous les étudiants */
    for (int i = 0; i < nb_etudiants; i++) {
        Etudiant* e = &etudiants[i];

        if (e->semestre_actuel >= semestre_min) {
            /* Classifier selon le statut */
            switch (e->etat) {
            case DEMISSION:
                if (e->semestre_actuel <= semestre_min + 1)
                    nb_demission++; /* Il est dans l'année */
                else
                    nb_passe++;  /* Il a dépassé */
                break;

            case DEFAILLANCE:
                if (e->semestre_actuel <= semestre_min + 1)
                    nb_defaillance++; /* Il est dans l'année */
                else
                    nb_passe++;  /* Il a dépassé */
                break;

            case EN_COURS:
                if (e->semestre_actuel <= semestre_min + 1)
                    nb_en_cours++;  /* Il est dans l'année */
                else
                    nb_passe++;     /* Il a dépassé */
                break;
            case TERMINE:
                nb_passe++;
            }
            switch (e->bilan){
            case AJOURNE:
                if (e->semestre_actuel == semestre_min || e->semestre_actuel == semestre_min + 1) {
                    nb_passe--;
                    nb_ajourne++;
                }
                break;
            }
        }
    }

    printf("%d demission(s)\n", nb_demission);
    printf("%d defaillance(s)\n", nb_defaillance);
    printf("%d en cours\n", nb_en_cours);
    printf("%d ajourne(s)\n", nb_ajourne);
    printf("%d passe(s)\n", nb_passe);
}