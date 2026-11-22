# XTOOLS pro Schulze ISL8

**XTOOLS** je nezávislý servisní a údržbářský nástroj vyvinutý pro **nabíječku baterií Schulze ISL8**.

Byl to první projekt vytvořený během reverzního inženýrství ISL8 a původně začínal jako nástroj pro přístup a správu kalibračních dat napětí nabíječky.

Na rozdíl od diagnostického nástroje založeného na PC běží **XTOOLS přímo na původním hardwaru ISL8**.

Obsahuje vlastní inicializační rutiny a k provozu nevyžaduje původní firmware Schulze.

---

## Přehled

Po spuštění provede XTOOLS inicializaci potřebnou k převzetí kontroly nad hardwarem ISL8.

To zahrnuje:

* inicializaci hlavního CPU
* inicializaci a konfiguraci LCD
* inicializaci požadovaných periferií
* konfiguraci hardwaru nabíječky
* uvedení napájecích obvodů nabíječky do bezpečného klidového stavu
* inicializaci paměťových rozhraní

Po dokončení inicializace hardwaru se spustí servisní prostředí XTOOLS.

Program poté poskytuje přímý přístup k vybraným servisním a údržbovým funkcím nabíječky.

---

## Kalibrační data

XTOOLS byl původně vyvinut pro zkoumání a servis kalibračního systému napětí ISL8.

Nástroj dokáže:

* číst existující kalibrační hodnoty
* zobrazovat informace o kalibraci
* přistupovat ke kalibračním datům uloženým v paměti Timekeeperu
* zapisovat kalibrační hodnoty zpět do Timekeeperu

To umožnilo zkoumat kalibrační systém nezávisle na původním firmwaru nabíječky.

---

## Servis Timekeeperu

ISL8 používá paměť Timekeeperu pro trvalá konfigurační a provozní data.

XTOOLS poskytuje servisní funkce pro tuto paměť, včetně:

* čtení uložených dat
* zápisu vybraných dat
* mazání obsahu Timekeeperu
* přípravy náhradního Timekeeperu k instalaci

To je obzvláště užitečné, když původní Timekeeper selhal nebo musí být vyměněn.

### Inicializace nového Timekeeperu

XTOOLS poskytuje postup pro správnou přípravu nového Timekeeperu k použití v nabíječce ISL8.

Během tohoto procesu nástroj obnoví informace požadované nabíječkou a rekonstruuje sériové číslo nabíječky pomocí dat uložených samostatně v paměti EEPROM na základní desce ISL8.

Díky tomu je XTOOLS důležitým servisním nástrojem při výměně vadného zařízení Timekeeper.

---

## Služba uživatelských dat

XTOOLS také poskytuje funkce údržby pro vybraná data uživatelské konfigurace.

Mezi dostupné funkce patří:

* reset hesla uživatele
* reset uživatelského jména
* vymazání vybraných trvalých uživatelských dat

Tyto funkce mohou být užitečné při servisu nabíječky, u které již není známa původní konfigurace uživatele.

---

## Samostatný provoz

Jednou z důležitých charakteristik XTOOLS je, že se jedná o **samostatné firmwarové prostředí**.

Pro provoz XTOOLS není vyžadován původní firmware Schulze ISL8.

XTOOLS provádí vlastní:

1. Inicializaci CPU
2. Inicializaci LCD
3. Inicializaci hardwaru
4. konfiguraci v bezpečném stavu
5. inicializaci paměti
6. spuštění servisní aplikace

ISL8 proto efektivně funguje jako vyhrazený servisní hardware XTOOLS, když je nástroj aktivní.

---

## Hardwarový adaptér XTOOLS

Projekt obsahuje vyhrazený hardwarový adaptér, který umožňuje ISL8 přepínat mezi původní ROM nabíječky a ROM XTOOLS.

Adaptér umožňuje zachovat normální provoz nabíječky a zároveň v případě potřeby poskytovat přístup k nezávislému servisnímu prostředí XTOOLS.

Spínací systém je řízen **mikrokontrolérem PIC**.

---

## Obsah repozitáře

Tento adresář obsahuje kompletní projekt XTOOLS, včetně:

### Firmware XTOOLS

Kompletní zdrojový projekt pro firmware XTOOLS.

Firmware byl vyvinut pomocí vývojového prostředí **Keil**.

### Adaptér XTOOLS

Kompletní hardwarový projekt pro adaptér XTOOLS používaný s nabíječkou ISL8.

### Výrobní data pro desku plošných spojů

Výrobní soubory pro desku plošných spojů adaptéru XTOOLS, včetně dat Gerber potřebných pro výrobu desky plošných spojů.

### Řídicí jednotka přepínače PIC

Zdrojové/projektové soubory pro mikrokontrolér PIC zodpovědný za přepínání mezi:

* původní ROM ISL8
* ROM XTOOLS

To umožňuje nabíječce normálně fungovat s původním firmwarem nebo se spustit do XTOOLS pro servis.

---

## Účel projektu

XTOOLS byl vytvořen primárně jako **nástroj pro opravu, kalibraci, konzervaci a reverzní inženýrství** stávajících nabíječek Schulze ISL8.

Projekt demonstruje, že hardware ISL8 lze inicializovat a provozovat nezávisle na původním firmwaru a poskytuje přímý přístup k servisním funkcím, které jsou užitečné při údržbě stárnoucího hardwaru.

XTOOLS může pomoci zejména s:

* analýzou kalibračních dat
* zálohováním a obnovou kalibračních dat
* výměnou časoměřiče
* inicializací časoměřiče
* obnovou identifikačních dat nabíječky
* obnovením/resetem hesla
* resetem uživatelské konfigurace
* výzkumem hardwaru a reverzním inženýrstvím

---

## Důležité: Zálohujte původní data

Před úpravou nebo vymazáním časoměřiče vždy vytvořte zálohu původního obsahu, kdykoli je to možné.

Některá data uložená v nabíječce mohou být pro dané zařízení jedinečná.

Nemažte ani nepřepisujte původní paměť, dokud nebude vytvořena ověřená záloha.

---

## Bezpečnostní varování

XTOOLS pracuje přímo s hardwarem nabíječky baterií.

I když XTOOLS inicializuje hardware nabíječky do klidového stavu před vstupem do servisní aplikace, úpravy kalibračních dat nebo perzistentní paměti mohou ovlivnit následný provoz nabíječky.

Nesprávná kalibrační data mohou způsobit nesprávné měření napětí a mohou vést k nebezpečnému nabíjení baterie.

Po úpravě kalibrace nebo výměně perzistentní paměti by měla být nabíječka před návratem do normálního provozu plně ověřena a kalibrována.

Tento projekt používejte pouze v případě, že rozumíte hardwaru a důsledkům úpravy jeho perzistentní konfigurace.

---

## Stav vývoje

XTOOLS je funkční projekt vyvinutý reverzním inženýrstvím hardwaru Schulze ISL8.

Další dokumentace bude přidána, jakmile budou historické vývojové soubory, dokumentace k hardwaru a zdrojový kód organizovány pro veřejné vydání.

---

## Prohlášení o vyloučení odpovědnosti

XTOOLS je nezávislý projekt.

Není to oficiální produkt společnosti Schulze Elektronik a není přidružen k společnosti Schulze Elektronik, není jí sponzorován ani schválen.

Názvy produktů Schulze se používají pouze k identifikaci hardwaru, pro který byl tento servisní nástroj vyvinut.