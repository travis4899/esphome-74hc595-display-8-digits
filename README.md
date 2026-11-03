# ESPHome 74HC595 8-Digit Display

Composant externe pour [ESPHome](https://esphome.io/) destiné au pilotage d'un afficheur 7 segments de **8 digits**, réalisé à partir de deux registres à décalage **74HC595** montés en cascade.

Le projet vise à fournir une solution réutilisable, configurable depuis le YAML ESPHome et indépendante du matériel spécifique d'un projet donné.

> **État du projet :** initialisation. Le composant n'est pas encore implémenté. Le brochage, la stratégie de multiplexage et les limites électriques seront validés au fil des essais.

## Objectif

Développer un composant ESPHome capable de piloter un afficheur 8 digits avec seulement trois GPIO de l'ESP32 :

- **DATA** : données série envoyées aux registres ;
- **CLOCK** : horloge de décalage ;
- **LATCH** : validation simultanée des sorties des registres.

Les deux 74HC595 sont chaînés : les données traversent le premier registre puis le second. Le composant devra gérer le rafraîchissement de l'afficheur et le multiplexage des digits sans bloquer les autres tâches d'ESPHome.

## Matériel cible

- Un afficheur 7 segments de 8 digits ;
- Deux circuits **74HC595** (par exemple SN74HC595N) ;
- Un ESP32 compatible ESPHome ;
- Des résistances de limitation de courant pour les segments ;
- Si nécessaire, des transistors ou drivers externes pour la sélection des digits.

### Architecture envisagée

Les deux registres auront des fonctions distinctes :

1. **Registre segments** : commande des segments A à G et du point décimal ;
2. **Registre digits** : sélection du digit actif parmi les huit positions.

Le composant effectuera un multiplexage : il sélectionnera un digit, appliquera le motif de segments correspondant, puis passera rapidement au digit suivant. La persistance visuelle donnera l'impression que les huit digits sont allumés simultanément.

L'affectation exacte des registres dans la chaîne, l'ordre des bits, la polarité des sorties et le câblage commun-anode / commun-cathode restent à confirmer sur le matériel réel.

## Fonctionnalités visées

- Affichage de valeurs numériques sur 8 digits ;
- Gestion des chiffres de 0 à 9 ;
- Gestion du signe négatif ;
- Gestion du point décimal ;
- Contrôle de l'alignement et du remplissage par zéros ou espaces ;
- Réglage de la fréquence de multiplexage ;
- Gestion de la luminosité, si elle peut être réalisée proprement avec l'architecture retenue ;
- Configuration complète depuis le YAML ESPHome ;
- Rafraîchissement non bloquant, compatible avec le fonctionnement normal d'ESPHome.

Les fonctionnalités seront ajoutées progressivement, après validation du fonctionnement électrique et temporel de la version minimale.

## Configuration YAML envisagée

La syntaxe ci-dessous est indicative. Elle sera ajustée lors de l'implémentation du composant.

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/travis4899/esphome-74hc595-display-8-digits
      ref: main
    components: [ display_74hc595 ]

display:
  - platform: display_74hc595
    id: afficheur_8_digits
    data_pin: GPIO16
    clock_pin: GPIO18
    latch_pin: GPIO17
    # Paramètres à définir après les premiers essais
```

L'API d'affichage sera définie pour s'intégrer naturellement au modèle des composants `display` d'ESPHome, avec notamment une lambda `it.print()` ou `it.printf()` si cela correspond à l'architecture retenue.

## Étapes de développement

### 1. Étude et validation du matériel

- [ ] Identifier précisément l'afficheur et son type électrique ;
- [ ] Déterminer le brochage des deux 74HC595 et leur ordre dans la chaîne ;
- [ ] Identifier la correspondance entre les sorties Q0–Q7 et les segments / digits ;
- [ ] Déterminer les niveaux actifs et les éventuelles inversions ;
- [ ] Vérifier les courants admissibles et définir les éventuels étages de commande externes.

### 2. Prototype bas niveau

- [ ] Écrire un test minimal de décalage de 16 bits ;
- [ ] Valider DATA, CLOCK et LATCH ;
- [ ] Commander indépendamment les sorties des deux registres ;
- [ ] Afficher un motif fixe sur un digit ;
- [ ] Valider la sélection successive des huit digits.

### 3. Composant externe ESPHome minimal

- [ ] Définir l'arborescence du composant ;
- [ ] Implémenter la configuration YAML et la validation des paramètres ;
- [ ] Implémenter le transfert des données vers les deux registres ;
- [ ] Mettre en place le multiplexage non bloquant ;
- [ ] Obtenir un affichage stable des huit digits.

### 4. API d'affichage

- [ ] Intégrer le composant à la plateforme `display` d'ESPHome ;
- [ ] Ajouter l'affichage des chiffres et des nombres ;
- [ ] Ajouter le signe négatif et le point décimal ;
- [ ] Définir le comportement en cas de dépassement de capacité ;
- [ ] Tester les différentes valeurs et transitions.

### 5. Configuration et robustesse

- [ ] Exposer les paramètres utiles dans le YAML ;
- [ ] Vérifier la stabilité du multiplexage sur la durée ;
- [ ] Vérifier la coexistence avec Wi-Fi, API, OTA et les autres composants ESPHome ;
- [ ] Documenter les limites électriques et les variantes de câblage ;
- [ ] Fournir des exemples complets de configuration.

### 6. Documentation et publication

- [ ] Finaliser le README et les schémas de câblage ;
- [ ] Ajouter des exemples ESPHome fonctionnels ;
- [ ] Documenter l'installation du composant externe ;
- [ ] Définir une première version stable.

## Organisation du développement

Le développement se fait directement sur la branche `main`.

Chaque étape doit être testée sur le matériel réel avant de passer à la suivante. Les choix d'architecture qui ne sont pas encore validés seront documentés comme tels, plutôt que présentés comme des caractéristiques définitives.

## Sécurité électrique

Un 74HC595 ne doit pas être considéré comme un driver de puissance. Les courants des segments et des digits doivent respecter les caractéristiques électriques du circuit et de l'afficheur. Des transistors ou drivers externes pourront être nécessaires, notamment pour la sélection des digits.

Ne pas alimenter directement les segments ou les communs de l'afficheur sans avoir vérifié les courants et les limites des composants.

## Licence

Licence à définir.
