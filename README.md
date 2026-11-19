# ESPHome 74HC595 8-digit display

Composant externe ESPHome pour les afficheurs 7 segments de 8 chiffres pilotés par deux 74HC595.

## État

Le premier pilote SPI est en place pour **un afficheur à la fois**. Il envoie les motifs des segments et la sélection du chiffre par SPI, puis rafraîchit un chiffre par passage dans `loop()`. Le rafraîchissement est réglé à 100 µs par chiffre, soit environ 1 250 images par seconde pour les huit chiffres.

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

Branchement de ce test : `GPIO23 → DIO`, `GPIO18 → SCK`, `GPIO19 → RCK`, alimentation `5V` et masse commune avec l’ESP32. Les broches choisies sont dédiées à cet afficheur ; le 74HC595 des six LED reste sur son câblage actuel.

La chaîne de texte est alignée à droite et limitée à huit caractères. Les chiffres, le signe moins et le point décimal sont pris en charge. Par exemple, `"1985"` occupe les quatre positions les plus à droite.

## Pilote initial

- SPI mode 0, MSB first, 1 MHz ;
- transfert du motif segments puis du masque de chiffre ;
- segments actifs à l’état bas et sélection du chiffre active à l’état haut par défaut ;
- remappage physique des chiffres `4,5,6,7,0,1,2,3`, selon la disposition décrite par AceSegment pour les modules RobotDyn/diymore 8 chiffres ;
- une lambda de texte évaluée à l’intervalle configuré ; le balayage continue indépendamment dans `loop()`.

Les options `segment_active_low` et `digit_active_high` permettent d’inverser les polarités dans le YAML. Ce pilote n’a pas encore été vérifié sur une platine réelle. Avant d’ajouter d’autres afficheurs ou de partager un bus avec d’autres registres, valider d’abord les chiffres et la polarité sur un module.

## Sécurité électrique

Vérifier les niveaux logiques acceptés par les entrées du module alimenté en 5 V et les courants admissibles des segments et des 74HC595. Relier la masse de l’ESP32 à celle de la platine.
