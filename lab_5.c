```
#define do_ 131
#define do_diez 139
#define re 147
#define re_diez 156
#define mi 165
#define fa 175
#define fa_diez 185
#define sol 196
#define sol_diez 208
#define la 220
#define la_diez 233
#define si 247

#define Do 262
#define Do_diez 277
#define Re 294
#define Re_diez 311
#define Mi 330
#define Fa 349
#define Fa_diez 370
#define Sol 392
#define Sol_diez 415
#define La 440
#define La_diez 466
#define Si 494

#define DO 523
#define DO_diez 554
#define RE 587
#define RE_diez 622
#define MI 659
#define FA 698
#define FA_diez 740
#define SOL 784
#define SOL_diez 831
#define LA 880
#define LA_diez 932
#define SI 988

#define re1 147
#define mi1 165
#define fa1 175
#define sol1 196
#define la1 220
#define sib1 233 // Si bemol
#define do1 262
#define re1_oct 294

// Octava 2 (medie)
#define re2 294
#define mi2 330
#define fa2 349
#define sol2 392
#define la2 440
#define sib2 466 // Si bemol
#define do2 523
#define re2_oct 587

// Octava 3 (înaltă)
#define re3 587
#define mi3 659
#define fa3 698
#define sol3 784
#define la3 880
#define sib3 932 // Si bemol
#define do3 1047
#define re3_oct 1175

    void setup()
{

    for (int i = 4; i <= 7; i++)
    {
        pinMode(i, OUTPUT);
        digitalWrite(i, LOW);
    }

    int note[] = {
        la1, 8,
        re1_oct, 12, mi2, 4, fa2, 8, sol2, 8,
        la2, 16, re1_oct, 8, re1_oct, 8,
        sib2, 12, la2, 4, sol2, 8, sol2, 8,
        do2, 12, sib2, 4, la2, 8, la2, 8,
        re2_oct, 12, do2, 4, sib2, 4, la2, 4, sol2, 8,
        la2, 8, fa2, 16, sol2, 8,
        la2, 12, sol2, 4, fa2, 8, mi2, 8,

        re2, 12, mi2, 4, fa2, 8, sol2, 8,
        la2, 8, sol2, 4, fa2, 4, mi2, 8, fa2, 8,
        re2, 24, re2, 8,
        re2, 12, re3_oct, 4, re3_oct, 8, re3_oct, 8,
        re3_oct, 16, do2, 8, do2, 8,
        re3_oct, 12, do2, 4, sib2, 4, la2, 4, sol2, 8,
        do2, 12, sib2, 4, la2, 8, la2, 8,
        re3_oct, 12, do2, 4, sib2, 4, la2, 4, sol2, 8,
        la2, 8, fa2, 16, sol2, 8};

    int octava = 1;
    int lungime = sizeof(note) / sizeof(note[0]) / 2;

    for (int i = 0; i < lungime; i++)
    {
        int durata = 100 * note[i * 2 + 1];

        tone(4, note[i * 2] * octava, 100 * note[i * 2 + 1]);
        delay(durata * 1.05); // doar o mică pauză între note
        noTone(4);
    }
}

void loop() {}

```