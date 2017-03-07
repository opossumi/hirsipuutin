# Hirsipuutin

Yksinkertainen hirsipuupeli automaattivastustajille.

## Käyttö

### Interaktiivinen käyttö

Aja hirsipuutin komennolla `python hirsipuutin.py sanat.txt`

### Ratkaisimen käyttö

Aja hirsipuutin komennolla `python hirsipuutin.py sanat.txt <ratkaisimen ajokomento>`, esimerkiksi `python hirsipuutin.py sanat.txt python ratkaisin.py`

### Asetukset

* `--count N`
  
    Suorittaa N pelin peluutuksen. Jos N=0, peluutetaan kaikki sanaston sanat.
  
* `--seed X`

    Valitsee sanojen satunnaisvalinnassa käytetyn satunnaistuksen siemenluvun X. Voidaan käyttää saman sanavalintajoukon toistamiseen erillisillä ajokerroilla.

* `-v`, `-vv`

    Lisää tulostettavan lokituksen määrää ratkaisinta käytettäessä. `-v` tulostaa arvaukset, tilarivit ja tulosrivit. `-vv` tulostaa myös sanaston.

* `--skip-dictionary`

    Ohittaa sanaston lähettämisen pelaajalle. Suositellaan vain interaktiiviseen käyttöön.

* `--save-score-data`

    Tallentaa pelaajan pistekertymän tiedostoon `<pelaajan nimi>.<juokseva numero>.data`
* `--server`

    Käynnistää sovelluksen palvelintilassa, peluuttaa hirsipuuta WebSocketin yli

* `--`

    Merkitsee ratkaisimen ajokomennon alkamisen eksplisiittisesti. Esimerkiksi `python hirsipuutin.py words.txt -v -- python ratkaisin.py --server` antaa server-vivun ratkaisimelle, ei hirsipuuttimelle. Ilman `--`-vipua hirsipuutin käynnistyisi palvelintilaan.

## Protokolla

Kaikki teksti hirsipuuttimen ja ratkaisimen välillä välitetään UTF-8 -koodauksella. Viestit päättyvät rivinvaihtoon.

1. Aluksi pelaaja syöttää nimensä
1. Saatuaan pelaajan nimen hirsipuutin syöttää pelaajalle käytetyn sanaston, jokainen sana omalla rivillään. Tyhjä rivi merkitsee sanaston loppumista.
1. Tämän jälkeen hirsipuutin syöttää pelaajalle tilarivin, joka signaloi joko kesken olevan sanan, voiton tai tappion.
  1. Voiton/häviön tapauksessa peli päättyy. Jos peluutetaan useampi kierros, tulostetaan seuraavan sanan ensimmäinen tilarivi.
1. Seuraavaksi pelaaja syöttää arvauksensa kirjaimelle (pienet kirjaimet) tai koko sanalle.
1. Hirsipuutin reagoi tähän syöttämällä pelaajalle tulosrivin, joka merkitsee joko onnistumista tai epäonnistumista.

Tilarivi on voiton tapauksessa muotoa `WIN <oikeiden arvausten lukumäärä>/<väärien arvausten lukumäärä>/<pistekertymä> <sana>`, tappion tapauksessa `LOSE <oikeiden arvausten lukumäärä>/<väärien arvausten lukumäärä>/<pistekertymä> <sana>`. Jos sana on kesken, tilarivi sisältää sanan arvaamattomat kirjaimet korvattuina pisteillä. Arvatut kirjaimet esitetään aina pieninä kirjaimina.

Tulosrivi on onnistumisen tapauksessa muotoa `HIT <oikeiden arvausten lukumäärä>/<väärien arvausten lukumäärä> <arvattu kirjain/sana>` ja epäonnistumisen tapauksessa `MISS <oikeiden arvausten lukumäärä>/<väärien arvausten lukumäärä> <arvattu kirjain/sana>`.
