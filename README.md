# Home Assistant SmartMonitor
![](https://raw.githubusercontent.com/PM04290/Home-Assistant-SmartMonitor/main/res/flashscreen.png)

Version alpha 0.1

## Présentation

SmartMonitor est un logiciel embarqué dans un ESP32 permettant de communiquer avec Home Assistant, il sera vu comme un "device".

```seq
Home Assistant->SmartMonitor: Publish sensors state
SmartMonitor->>Home Assistant: DeviceTriggers
SmartMonitor->>Home Assistant: Command Cover, switch,...
SmartMonitor->>Home Assistant: Alarm control panel
Note right of SmartMonitor: Configuration\nby\nweb serveur
Note right of SmartMonitor: sensor/switch\non GPIO
Home Assistant-->SmartMonitor: Device tracker
```

##Programmation de l&apos;ESP32

L&apos;explication ci-dessous ne tient compte que de l&apos;utilisation de l&apos;IDE  Arduino.

###Préparation
Vous devez avoir installé le core 1.x pour ESP32 en ajoutant le lien ci-dessous dans les *préférences*:
https://dl.espressif.com/dl/package_esp32_index.json

Le core 2.x sera utilisé dans les prochains mois afin de pouvoir  utiliser les ESP32-C et S.

Pour télécharger le dossier *data*  vous devez avoir installé le plug-in ci-dessous:
https://github.com/me-no-dev/arduino-esp32fs-plugin

Il exsiste de très bons tutoriels si vous avez des problèmes sur ces deux points.

###WT32-SC01
Si vous pocédez le module WT32-SC01, vous pouvez télécharger directement le [firmware](http://todo/) sans avoir à tout recompiler ni vous préaucuper des librairies.
Il est disponible chez [aliexpress](https://fr.aliexpress.com/item/1005003243416096.html) ou chez le [fabricant](https://french.alibaba.com/product-detail/WT32-SC01-16MB-esp32-touch-screen-1600308289988.html).
![](http://www.wireless-tag.com/wp-content/uploads/2020/07/%E6%9C%AA%E6%A0%87%E9%A2%98-1-%E6%81%A2%E5%A4%8D%E7%9A%84-300x263.jpg)

###ESP32 WROOM  ou WROVER

####Dépendences des libraires

- Disponibles dans le gestionnaire de librairies:
>ArduinoJson
ESPAsyncWebServer
LovyanGFX
- En téléchargmenet sur mon Github
>HAintegration [ici](http://todo/)

####Préparation du module

Avant de compiler le projet il faut configurer celui-ci pour votre couple ESP / TFT.
pour celà éditez le fichier display_setup.h; selectionnez le define ESP32_TFT:
```c
// Choix de la cible
//#define SC01
//#define SC01Plus
#define ESP32_TFT
```
Puis en bas du fichier, il faut configurer le type de *Panel*, de *Bus*  et de *TouchScreen* .

```cpp
class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ILI9341 _panel_instance;
    lgfx::Bus_Parallel8 _bus_instance;
    lgfx::Touch_XPT2046 _touch_instance;
```

Ensuite il faut indiquer le raccordement qui est fait sur l&apos;ESP32, l&apos;exemple ci-dessous est donnée pour un écran TFT équipé du bus de data  8bits.

```cpp
        auto cfg = _bus_instance.config();
        cfg.freq_write = 20000000;
        cfg.pin_wr = 4;
        cfg.pin_rd = 2;
        cfg.pin_rs = 15;  // Data / Command

        // LCD data interface, 8bit MCU (8080)
        cfg.pin_d0 = 12;
        cfg.pin_d1 = 13;
        cfg.pin_d2 = 26;
        cfg.pin_d3 = 25;
        cfg.pin_d4 = 17;
        cfg.pin_d5 = 16;
        cfg.pin_d6 = 27;
        cfg.pin_d7 = 14;

        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
```
Puis la configuration du Panel

```cpp
        auto cfg = _panel_instance.config();

        cfg.pin_cs           =    33;
        cfg.pin_rst          =    32;
        cfg.pin_busy         =    -1;

        cfg.panel_width      =   240;
        cfg.panel_height     =   320;
        cfg.offset_x         =     0;
        cfg.offset_y         =     0;
        cfg.offset_rotation  =     1;
        cfg.dummy_read_pixel =     8;
        cfg.dummy_read_bits  =     1;
        cfg.readable         =  true;
        cfg.invert           = false;
        cfg.rgb_order        = false;
        cfg.dlen_16bit       = false;
        cfg.bus_shared       =  true;

        _panel_instance.config(cfg);
```
Si le TFT est équipé du TouchScreen, il est possible de le configurer, sinon la définition peut être supprimée (ou commentée).

```cpp
        auto cfg = _touch_instance.config();

        cfg.x_min      = 0;
        cfg.x_max      = 239;
        cfg.y_min      = 0;
        cfg.y_max      = 319;
        cfg.pin_int    = -1;
        cfg.bus_shared = true;
        cfg.offset_rotation = 1;

        cfg.spi_host = VSPI_HOST;
        cfg.pin_cs   = 33;
        cfg.pin_mosi = GPIO_NUM_23;
        cfg.pin_miso = GPIO_NUM_19;
        cfg.pin_sclk = GPIO_NUM_18;

        cfg.freq = 2700000;

        _touch_instance.config(cfg);
        _panel_instance.setTouch(&_touch_instance);
```

De même si un rétroéclairage est disponible, vous pouvez vous baser sur la définition du SC01 pour le configurer, par exemple:

```cpp
        auto cfg = _light_instance.config();

        cfg.pin_bl = 35;
        cfg.invert = false;
        cfg.freq   = 44100;
        cfg.pwm_channel = 7;

        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance);
```

Sans oublier d&apos;ajouter le déclaration au début de Class:

```cpp
    lgfx::Light_PWM     _light_instance;
```

N&apos;hésitez pas à vous repporter au site original de la librairie [LovyanGFX](todo) afin de trouver des informations.

Si vous souhaitez utiliser des pin libres pour gérer des Sensor ou des Switch, il faut indiquer leur numéro dans le tableau suivant:
```cpp
static uint8_t pinAvailable[] = {5, 22, 23};
```

Cette partie est, pour le moment, la plus compliquée du projet en attendant que la banque de données s&apos;enrichisse d&apos;une liste d&apos;écran utilisés.


Compilez le projet, téléchargez le dossier *data* et téléversez le projet.

Les *data* contiennent une configuration par défaut qui affiche uniquement l&apos;élévation du soleil car l&apos;intégration est normalement présente sur toutes les installations H.A.
Cette configuration ne contient pas les caractéristiques de votre wifi, il faut donc établir une connexion au serveur web du module.
Quand le module ne trouve pas de réseau wifi, il passe automatiquement en mode *Point d&apos;accès*; vous devez donc vous connecter à son wifi propriétaire : **smartmon0**
Le mot de passe par défaut est : 12345678

Ensuite, utilisez un navigateur internet sur l&apos;url http://smartmon0.local afin d&apos;entrer les premiers paramètres de configuration (l&apos;adresse IP par défaut est 192.168.4.1)

La page web propose 3 zones:
- Pages / Actionneurs
- Wifi
- Mises à jour (firmware et fichiers)

Commencez par configurer le réseau wifi ainsi que les caractéristiques d&apos;accès au serveur mqtt de H.A.

>Si vous avez un serveur DHCP qui contrôle les adresses MAC, celle du module est indiquée dans le titre de la zone afin d&apos;ouvrir le bail adapté sur votre serveur DHCP.

⚠️ Important
>Si vous devez utiliser pluiseurs modules, par exemple 2, indiquez 1 dans Code. Après le redémarrage,  le module répondra à l&apos;adresse **smartmon1.local** et ainsi le module suivant ne sera pas être perturbé.

La première configuration peut se faire avec un téléphone portable, la page web est 'responsive'.

C&apos;est aussi lors de cette configuration que vous pouvez choisir l&apos;orientation de l&apos;afficheur : Vertical / Horizontal.

Après la préparation du module, avec la configuration du wifi et du serveur Mqtt, si vous démarrez vous devriez avoir une icône d&apos;erreur et l&apos;informaton de l&apos;heure qui n&apos;arrive pas : H.A. n&apos;envoie pas encore de données.

###Préparation de H.A.

Avant tout il faut que H.A. publie les états des différentes entités que vous souhaitez gérer avec le module, et il faut commencer par l&apos;heure, afin de valider la bonne connexion du module au serveur.

####Mise à jour de l&apos;heure　
Créez une automatisation :
```yaml
- id: sm_mqtt_publish_time #id uniquement si vous éditez le fichier .yaml
  alias: Publication heure pour SmartMonitor
  description: ''
  trigger:
  - platform: time_pattern
    minutes: /1
  condition: []
  action:
  - service: mqtt.publish
    data:
      topic: smartmonitor/dateheure
      payload_template: '{{ now().strftime(''%d/%m/%Y %H:%M'') }}'
  mode: single
```
Une fois l&apos;automatisation activée, l&apos;heure est publiée toutes les minutes; si elle ne s&apos;affiche pas sur le module il faut vérifier la configuration Mqtt : Adresse, utilisateur et mot de passe.

>A savoir que le module ne fait aucune interprétation du texte qui est envoyé, vous pouvez vous servir de se Topic pour envoyer n&apos;importe quel texte qui sera affiché en haut au centre de l&apos;écran.

####Publication des *sensors*

Pour réaliser celà il faut utiliser le [mqtt_statestream](https://www.home-assistant.io/integrations/mqtt_statestream/) de H.A.

Dans le fichier configuration.yaml il faut ajouter :
```yaml
mqtt_statestream:
  base_topic: smartmonitor # Ne pas changer
  publish_attributes: true     # Obligatoire
  include:
    domains:
      - sun
```
Commençons simplement par le domaine sun qui va permettre de valider la configuration présente dans le module.

Après le redémarrage de H.A. vous devriez voir l&apos;élévation sur le module.

⚠️ le `base_topic` ne doit pas être changé et la ligne `publish_attributes: true`  est obligatoire car elle permet au module de récupérer les informations complémentaires des capteurs : unité, type, etc.

Basez vous sur la documentation de Home Assistant afin de régler correctement la diffusion des informations utilles à SmartMonitor. Les balises `include` / `exclude` ainsi que `domains` et `entities` permettent un réglage très fin de la diffusion.

Voici un exemple trivial, non filtré, de la publication des toutes les entités des domaines indiqués:

```yaml
mqtt_statestream:
  base_topic: smartmonitor
  publish_attributes: true
  include:
    domains:
      - weather
      - sensor
      - binary_sensor
      - cover
      - switch
      - light
      - sun
      - alarm_control_panel
```
Sur une grosse configuration, le serveur Mqtt va être chargé de beaucoup de données pas nécéssairement utiles au module.

N&apos;hésitez pas à utiliser [Mqtt Explorer](http://mqtt-explorer.com/) afin de visualiser la liste des données publiées.

>A noter que si vous avez publié trop de données (comme l&apos;exemple ci-dessus) avant de mieux filtrer votre besoin, il est possible avec MQtt explorer de supprimer l&apos;arborescence **smartmonitor** et de relancer le serveur Mqtt. L&apos;arborescence sera reconstituée avec uniquement les valeurs filtrées.

###Configuration de SmartMonitor

Maintenant que plusieurs données sont disponibles sur le serveur Mqtt, il est possible de configurer l&apos;affichage sur le module.

En premier lieu il faut choisir le nombre de Zone d&apos;affichage. Par défaut la cofiguration est de 2x3 (2 colonnes, 3 lignes) qui est la plus optimisée pour un afficheur 480x320, à l&apos;horizontale, avec des longueurs de texte raisonnables.

⚠️ Si vous souhaitez changer cette disposition, il faut le faire au tout début, **Valider** cette simple modification et relancez le module (bouton **Reset** en bas de page).

Vous constaterez que l&apos;affichage de la page est effectué en plusieurs fois, en effet, il peut y avoir beaucoup de données affichées si il y a plusieurs pages et la mémoire de l&apos;ESP32 (même si elle est importante) ne permet pas de traiter un gros volume; l&apos;affichage a donc été segmenté en parties élémentaires, et envoyée par Websocket (un peu de technique). Laissez bien la page s&apos;afficher compètement avant de réaliser le paramétrage.
>Problème connu : lors de certains affichages, la page peut ne pas se raffraichir complètement ou rester bloquée au début du chargement. Je n&apos;ai pas encore réussi à trouver d&apos;où venait le problème. Mais rassurez vous, en général un simple raffraichissement du navigateur suffit à recharger la page correctement.

####Description des paramètres

#####Définition d&apos;une page

`ID` est un code unique de page qui va permettre la gestion du changement de page (lecture seule).

`Titre` est un libellé simple qui permet de s'y retrouver durant la configuration (sera peut être utilisé dans le futur comme titre de page).

`Type` à ce jour seul ***normalPage*** est utilisable; keypadPage est visible mais non sélectionnable, la page du digicode est crée implicitement lors de la configuration de *Alarm_control_panel*.

#####Définition des zones d&apos;affichage

Le nombre de ligne disponible par page va dépendre  des nombres Col/Lig que vous avez sélectionné.

Les zones sont affichées dans l'ordre suivant:

|  |  |
| :------:| :-----:|
|zone 1|zone 2|
|zone 3|zone 4|
|zone 5|zone 6|

`Titre` il s&apos;agit du titre d&apos;un Bouton, du nom d&apos;un Trigger (visible dans H.A.) ou simplement d&apos;un libellé dans le cas d&apos;affichage simple.

`Action` détermine la fonction de la zone:
- `Rien` est utilisé quand la zone doit rester vide
- `HAtrigger` est utilisé quand on veut définir un trigger qui sera utilisable par une automatisation dans H.A.; le titre devient le `subType` (le nom) du trigger visible par H.A. avec un type `button_short_press`.
- `changePage`  permet de définir la zone comme un bouton qui va permettre d&apos;ouvrir une nouvelle page. Le cas particulier du digicode est traité dans le chapitre de l&apos;alarme.
- `Display` définit un simple affichage de valeur.
- `Command` définit une action possible, par exemple l&apos;ouverture/fermeture de volet ou la gestion d&apos;un switch.

`MQTT` permet de définir le topic qui sera utilisé pour récupérer les données; ou les envoyer.
`Type` définit la façon dont sont affichées les données. Les premiers de la liste avec le nom qui commence par ***Etat*** permet d&apos;afficher une icône correspondant au *State* de l&apos;entité suivi du *Titre* choisi. Les autres affichent une icône correspondant à l'entité suivi de la valeur reçue.
>L&apos;unité est pour le moment codés "en dur", mais dans la TODO list il est prévu de récupérer l&apos;information depuis le serveur Mqtt.

`Page` permet de choisir la page qui sera affichée lors du choix de l&apos;action *changePage*.
>Par defaut la liste ne contient que "kp"  (keypad), il faut créer une nouvelle page pour que le complément de liste soit mis à jour.

#####Exemples de configurations

Ci-dessous, dans MQTT, les termes en **gras** sont ceux que vous avez choisi dans H.A.

Affichage de la température extérieure fournie par une intégration météo:
- Titre : Météo
- Action : Display
- MQTT : smartmonitor/weather/**maison**/temperature
- Type : Température
- Page : *inactif*

Pilotage d&apos;un volet, ou groupe de volets:
- Titre : Volets nuit
- Action : Command
- MQTT : smartmonitor/cover/**volet_nuit**;/state;/set
- Type : Etat volet
- Page : *inactif*

>Vous noterez la présence de `;` dans MQTT qui permet de séparer la base du topic, le terme qui permet la lecture et celui qui permet l&apos;écriture. les 2 termes **state** et **set** sont vrais dans la quasi totalité des cas de figure. Utilsez MQtt explorer pour vérifier cela si vous rencontrez un problème de pilotage et/ou d&apos;affichage.

Trigger permettant l&apos;ouverture d&apos;un portail:
- Titre : Portail
- Action : HAtrigger
- MQTT : *inactif*
- Type : *inactif*
- Page : *inactif*

>Dans H.A. vous trouverez un déclencheur attaché au device `smartmonitor0` ayant le nom `button_short_press_Portail` sur lequl vous pourrez déclencher une automatisation.

Exemple de changement de page:
>Il faut d&apos;abord créer la nouvelle page en utilisant le bouton prévu à cet effet (en bas à droite de la zone Pages). Par exemple une page regroupant toutes les informations de consommation/production d&apos;énergie.

- Titre : Info énergie
- Action : changePage
- MQTT : *inactif*
- Type : *inactif*
- Page : p1

####Gestion de l&apos;alarme

- Titre : Alarme
- Action : changePage
- MQTT : smartmonitor/alarm_control_panel/**alarme**/state
- Type : Etat lalarme
- Page : p1

L&apos;alarme est matérialisée par une icône représantant son statut:
- Désactivée
- Activée absent
- Activée nuit
- Armement
- Déclenchement

>pour le moment, seuls les modes `ALARM_AWAY` et `ALARM_NIGHT` sont disponibles, les autres sont dans la TODO list.

Lorsque l&apos;on presse l&apos;icone de l&apos;alarme, une page s&apos;ouvre qui affiche un digicode ainsi que les boutons de changement de mode.

Du coté de H.A. il faut que l&apos;alarme soit configurer en tant que `manual`.

```yaml
alarm_control_panel:
  - platform: manual
    name: "Alarme"
    code: 1234
    code_arm_required: false
    # temps en seconde pour partir après mise en mache
    arming_time: 30
    # temps en seconde pour saisir le code pour désarmer
    delay_time: 60
    # temps en seconde du décenchement (pour la durée sirène par exemple)
    trigger_time: 30
```
Lorsque un bouton de changement de mode est préssé, un message Mqtt est envoyé contenant le l&apos;action demandée et le code, par exemple:
```json
{action: ARM_NIGHT, code=""}
```
ou:
```json
{action: DISARM, code="1234"}
```

####Automatisation du changement de mode

L&apos;automatisation est crée initialement dans son éditeur afin de pouvoir choisir simplement le device et son trigger correspondant : **button_short_press_AlarmKeypad**

Exemple simple de changement de mode sans vérification de capteur:

```yaml
mode: single
trigger:
  - platform: device
    domain: mqtt
    device_id: 3cdce4ea751b50e2f5fe1b94a1710430 # defini par UI
    type: button_short_press
    subtype: AlarmKeypad
    discovery_id: a4e57ce081cc button_short_press_AlarmKeypad
condition: []
action:
  - service: alarm_control_panel.{{ trigger.payload_json.action }}
    target:
      entity_id:
      - alarm_control_panel.alarme
    data:
      code: '{{ trigger.payload_json.code | int(0) }}'
```

Dans cet autre exemple ci-dessous, il y a une vérification de l&apos;état d&apos;un groupe de contact de fenêtre, l&apos;alarme n&apos;est activée que si toutes les fenêtres sont fermées; sinon un message d&apos;information est envoyé au module.

```yaml
- id: sm_trigger_alarm_change_mode # uniquement si édition du fichier yaml
  alias: 'Alarme: Changement de mode'
  description: ''
  trigger:
  - platform: device
    domain: mqtt
    device_id: 284cfad79d5e602d26418e26554a4d6b #défini par UI
    type: button_short_press
    subtype: AlarmKeypad
    discovery_id: a4e57cdf15e4 button_short_press_AlarmKeypad
  condition: []
  action:
  - if:
    - condition: or
      conditions:
      - condition: state
        entity_id: binary_sensor.contacts_ouvertures
        state: 'off'
      - condition: not
        conditions:
        - condition: state
          entity_id: alarm_control_panel.alarme
          state: disarmed
    then:
    - service: alarm_control_panel.{{ trigger.payload_json.action }}
      target:
        entity_id:
        - alarm_control_panel.alarme
      data:
        code: '{{ trigger.payload_json.code | int(0) }}'
    else:
    - service: input_text.set_value
      data:
        value: '{% set defauts = ["Fenêtre(s) ouverte(s):"] + expand(''binary_sensor.contacts_ouvertures'')
          | selectattr(''state'',''in'',[''on'',''open'',''unknown'']) | map(attribute=''name'')
          | list %} {%- if defauts | length > 1 -%} {%- set json = {"text": defauts}
          -%} {{ json }} {%- endif -%}
          '
      target:
        entity_id: input_text.notification_text
  mode: single
```
La configuration du message temporaire est décrite ci-dessous.


#####Messages temporaires sur l&apos;écran

Le module peut recevoir un message temporaire, par exemple pour informer de l&apos;état des capteurs lors d&apos;une mise en alarme (sui une fenêtre est restée ouverte).
Pour cela il faut créer un input_text :
> Paramètres / Appareils et services / Entrées

Créez une nouvelle entrée de type *Texte* que vous nommerez par exemple : **notification_text**

Lors que l&apos;on désire envoyer un message au module, il suffit d&apos;affecter une valeur à **notification_text**; une automatisation va publier le message et réinitialiser la valeur après un certain temps.

#####Automatisation de message temporaire

```yaml
- id: sm_raz_published_message  #id uniquement si vous éditez le fichier .yaml
  alias: raz message smartmonitor
  description: ''
  trigger:
  - platform: state
    entity_id:
    - input_text.notification_text
  condition:
  - condition: template
    value_template: '{{ states(''input_text.notification_text'')|length != 0 }}'
  action:
  - service: mqtt.publish
    data:
      topic: smartmonitor/alarm_message
      payload_template: '{{ states(''input_text.notification_text'') }}'
  - delay:
      hours: 0
      minutes: 0
      seconds: 2
      milliseconds: 0
  - service: input_text.set_value
    data:
      value: ''
    target:
      entity_id: input_text.notification_text
  - delay:
      hours: 0
      minutes: 0
      seconds: 1
      milliseconds: 0
  - service: mqtt.publish
    data:
      topic: smartmonitor/alarm_message
      payload_template: '{{ states(''input_text.notification_text'') }}'
  mode: single
```


###TODO list

- [x] ajouter tous les types de capteurs connus par H.A.
- [ ] récupération automatique des unités d&apos;affichage
- [ ] compléter la liste des écrans TFT dans display_setup.h
- [ ] affichage du forecast météo (icônes sur n jours)
- [ ] gérer tous les mode d'alarme supplémentaires (present, vacation, custom)
- [ ] amélioration du message temporation pour devenir une notification
- [ ]
- [ ] traduction

Fichier readme édité avec [MEditor.md](https://pandao.github.io/editor.md/index.html)