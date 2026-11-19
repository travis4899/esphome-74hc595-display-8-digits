# ESPHome 74HC595 8-digit display

Composant externe ESPHome pour les afficheurs 7 segments de 8 chiffres pilotés par deux 74HC595.

## État

Le pilote SPI gère un afficheur ou une chaîne de **jusqu’à trois afficheurs**. Il envoie les motifs des segments et la sélection du chiffre par SPI. Le balayage est exécuté par le timer haute résolution ESP-IDF en contexte tâche, indépendamment de la boucle ESPHome, avec une cible de 500 µs par chiffre (250 images/s). Une instance peut piloter jusqu’à trois afficheurs chaînés. Le journal affiche la cadence effectivement mesurée.

Le code s’inspire du fonctionnement documenté dans [AceSegment](https://github.com/bxparks/AceSegment), notamment du pilote `Hc595Module`. La carte de test identifiée par photo est un ESP-WROOM-32 classique (cible ESPHome `variant: esp32`). Le matériel d’affichage reste à valider : ordre des octets, polarités et correspondance des chiffres peuvent varier selon les fabricants.

## Configuration ESPHome

Le composant est une plateforme `text_sensor`. Sa lambda retourne la chaîne à afficher. L’état du capteur correspond aussi au texte affiché.

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/travis4899/esphome-74hc595-display-8-digits
      ref: main
    components: [hc595_7seg]

spi:
  clk_pin: GPIO18
  mosi_pin: GPIO23

text_sensor:
  - platform: hc595_7seg
    id: test_afficheur
    name: "Test afficheur 74HC595"
    cs_pin: GPIO19 # RCK / latch
    update_interval: 1s
    lambda: |-
      return std::string("12345678");
```

Pour trois afficheurs en cascade, ajoute `chain_length: 3` et retourne les textes dans l’ordre des cartes, de la première (DIO relié à l’ESP32) à la dernière, séparés par `|` :

```yaml
text_sensor:
  - platform: hc595_7seg
    id: afficheurs
    name: "Afficheurs 74HC595"
    cs_pin: GPIO19
    chain_length: 3
    update_interval: 1s
    lambda: |-
      return std::string("12|1985|2025");
```

Les connecteurs latéraux de certaines versions de ces platines répètent les signaux d’entrée et ne chaînent pas les données. Pour chaque liaison, relie la sortie série `QH'` (broche 9) du deuxième 74HC595 d’une carte au `DIO` de la suivante. Relie `SCK` et `RCK` en parallèle aux trois cartes, ainsi que l’alimentation 5 V et la masse commune. La dernière carte n’a pas besoin d’une sortie série. Vérifie la broche 9 sur la référence exacte de tes puces avant soudure.

Branchement de ce test : `GPIO23 → DIO`, `GPIO18 → SCK`, `GPIO19 → RCK`, alimentation `5V` et masse commune avec l’ESP32. Les broches choisies sont dédiées à cet afficheur ; le 74HC595 des six LED reste sur son câblage actuel.

La chaîne de texte est alignée à droite et limitée à huit caractères. Les chiffres, le signe moins et le point décimal sont pris en charge. Par exemple, `"1985"` occupe les quatre positions les plus à droite.

## Pilote initial

- SPI mode 0, MSB first, 1 MHz ;
- transfert du motif segments puis du masque de chiffre ;
- segments actifs à l’état bas et sélection du chiffre active à l’état haut par défaut ;
- remappage physique des chiffres `4,5,6,7,0,1,2,3`, selon la disposition décrite par AceSegment pour les modules RobotDyn/diymore 8 chiffres ;
- une lambda de texte évaluée à l’intervalle configuré ; le balayage est cadencé séparément par le timer ESP-IDF.

Les options `segment_active_low` et `digit_active_high` permettent d’inverser les polarités dans le YAML. Le timer vise 250 images/s ; vérifier la cadence mesurée sur la platine. Avant d’ajouter d’autres afficheurs ou de partager un bus avec d’autres registres, valider d’abord les chiffres et la polarité sur un module.

## Sécurité électrique

Vérifier les niveaux logiques acceptés par les entrées du module alimenté en 5 V et les courants admissibles des segments et des 74HC595. Relier la masse de l’ESP32 à celle de la platine.
