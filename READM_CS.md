# Archiv a projekt reverzního inženýrství společnosti Schulze Elektronik GmbH

Nezávislý projekt uchovávání, dokumentace a reverzního inženýrství věnovaný starším nabíječkám baterií, balancerům a souvisejícímu vybavení od společnosti **Schulze Elektronik GmbH**.

Účelem tohoto úložiště je uchovávat technické znalosti o těchto zařízeních a zpřístupňovat výsledky probíhajícího reverzního inženýrství veřejnosti.

Mnoho zařízení Schulze se dodnes používá, ale podrobné technické informace, servisní dokumentace a vývojové nástroje je stále obtížnější najít.

Tento projekt se pokouší zdokumentovat provoz těchto zařízení a poskytnout informace a nástroje, které mohou pomoci dalším nadšencům s jejich opravami, údržbou, studiem a dalším vývojem.

---

## Projekty

### Schulze ISL8-936G

Reverzní inženýrství rodiny nabíječek Schulze **ISL8-936G**.

Mezi současné oblasti výzkumu patří:

* Architektura procesoru Siemens 80C537
* Analýza firmwaru ROM
* Externí SRAM a časoměřič M48T08
* EEPROM 93LC46
* Sledování provádění firmwaru
* Zachycení a protokolování adresní sběrnice
* Rekonstrukce firmwaru
* Řídicí a zobrazovací rutiny LCD
* Dekódování klávesnice a I/O
* Sériová komunikace
* Tabulky příkazů a interní struktury firmwaru
* Měřicí systém
* Kalibrační rutiny z výroby
* Servisní a diagnostické funkce
* Hardwarová dokumentace
* Vývoj vlastních nástrojů pro reverzní inženýrství

Hlavní částí tohoto projektu je vývoj kompletního hardwarového/softwarového systému schopného monitorovat provádění firmwaru a rekonstruovat vnitřní provoz nabíječky.

---

### Schulze ISL6-636+

Reverzní inženýrství a dokumentace nabíječky Schulze ISL6-636+

Výzkum zahrnuje:

* procesor Siemens 80C535
* firmware ROM
* EEPROM 93LC46
* LCD kompatibilní s HD44780
* rozhraní klávesnice
* sériovou komunikaci
* firmwarové rutiny
* měřicí a kalibrační funkce
* hardwarovou dokumentaci

ISL6 poskytuje zajímavé srovnání s pozdější a podstatně složitější architekturou ISL8.

---

### XCAL pro ISL8

Výzkum a dokumentace kalibračního zařízení Schulze a automatických kalibračních postupů.

Tento výzkum byl úspěšně potvrzen a dokončen. XCAL má stejnou funkci jako původní zařízení Schulze.

Jedná se o hardwarový přepínač specifikovaných napětí pro kalibraci nabíječky. Tento testovací modul BETA byl sestaven, naprogramován a úspěšně otestován.

Cílem bylo porozumět původnímu kalibračnímu systému a reprodukovat funkcionalitu potřebnou ke kalibraci stávajícího zařízení Schulze.

---

### XTOOLS pro ISL8

Výzkum a dokumentace kalibračního zařízení Schulze a automatických kalibračních postupů.

Tento modul XTOOLS byl prvním funkčním softwarovým nástrojem pro čtení a zápis kalibrací uložených v časomíře M48T08 v ISL8-936G. První výzkum trval tři roky a odhalil způsob použití kalibračních konstant.

Byl vyroben a úspěšně otestován, fungoval na principu ROM adaptéru, s IO přepínačem ROM s XTOOLS a ROM s FW850EN.

Cílem bylo porozumět původnímu kalibračnímu systému a reprodukovat funkcionalitu potřebnou ke kalibraci stávajícího zařízení Schulze.

Původní samostatný XTOOLS zůstává součástí archivu. Pozdější integrovaná implementace XTOOLS byla také vyvinuta pro firmware ISL8, včetně úpravy kalibrace a servisních funkcí časomíry.

---

### ISL6 XTOOLS a GUI

Dokončené projekty vývoje XTOOLS a GUI pro rodinu ISL6 budou v tomto archivu zdokumentovány samostatně. Na jejich stránkách projektu budou popsány podporované modely, podrobnosti implementace a testování hardwaru.

---

### LipoProfiBal06

Hardwarová analýza balanceru baterií Schulze LipoProfiBal08 a LipoProfiBal14.

Tento projekt zahrnuje:

* reverzní inženýrství původního hardwaru
* analýzu měřicího systému
* balancování výkonového stupně
* řízení MOSFET
* obvody pro řízení izolovaných hradlů
* měření proudu
* architekturu napájecího zdroje
* komunikační rozhraní
* vývoj moderní náhradní desky plošných spojů (PCB)

Nová deska plošných spojů má zachovat principy fungování původního zařízení a zároveň používat komponenty, které jsou dnes dostupné.

---

## Výzkum tovární kalibrace

Některý firmware nabíječek Schulze obsahuje funkce pro tovární kalibraci a diagnostiku, které jsou pro uživatele obvykle nepřístupné.

Součástí tohoto projektu je prozkoumat tyto rutiny a zdokumentovat jejich fungování. Řada nabíječek ISL8 byla úspěšně vystavena a spuštěna jako tovární kalibrace.

V případech, kdy jsou nutné úpravy firmwaru, je preferovaným přístupem tohoto repozitáře poskytovat **záplaty a nástroje pro záplatování, spíše než dále distribuovat kompletní obrazy firmwaru chráněné autorskými právy**.

Uživatel by si měl stáhnout originální firmware ze svého vlastního zařízení a lokálně aplikovat zdokumentovanou úpravu.

Pro verze firmwaru ISL8 **8.10 DE, 8.10 EN, 8.50 DE a 8.50 EN** je k dispozici hardwarově testovaný **Opravář tovární kalibrace F2**. Umožňuje přístup k existujícímu postupu tovární kalibrace prostřednictvím F2, mění zobrazenou identifikaci firmwaru a aktualizuje datum a čas úpravy. Nástroj pro záplatování ověřuje SHA-256 originálního firmwaru dodaného uživatelem a dále nedistribuuje obrazy firmwaru Schulze.

---

## Autor

**Martin Stránský (Prometheuss80)**

Nezávislý výzkum, reverzní inženýrství firmwaru, vývoj, analýza a testování hardwaru.

Copyright © 2026 Martin Stránský (Prometheuss80).

GitHub: [Prometheuss80](https://github.com/Prometheuss80)

---

## Licencování

Původní nástroje a kód vytvořený pro tento archiv mohou být vydány pod licencí **MIT**; přesný rozsah naleznete v souboru `LICENSE` nebo v licenci uvedené v každém adresáři projektu.

Licence MIT se **nevztahuje** na originální firmware Schulze Elektronik ani na materiály třetích stran, pokud to jejich příslušní držitelé práv neschválí. Jednotlivé adresáře projektů mohou mít své vlastní licenční podmínky.

---

## Filozofie repozitáře

Toto repozitář je primárně určen jako **technický archiv a zdroj pro společný výzkum**.

Projekt vyvíjím v mém volném čase a není možné, abych se mohl/a věnovat všem oblastem najednou.

Každý, kdo má zájem o konkrétní část projektu, je vítán, aby si prostudoval dostupné materiály, vytvořil fork a pokračoval ve výzkumu samostatně.

Užitečné objevy, opravy, dokumentace a vylepšení jsou vítány.

Není nutné rozumět celému systému. Jednotlivé oblasti, jako je analýza firmwaru, sestavení 8051, analogová elektronika, návrh plošných spojů, kalibrace, komunikační protokoly nebo softwarové nástroje, lze prozkoumat samostatně.

---

## Příspěvky

Příspěvky jsou vítány.

Můžete:

* forknout repozitář
* prozkoumat jakýkoli hardware nebo firmware
* vylepšit dokumentaci
* vyvíjet analytické nástroje
* ověřit stávající znalosti
* odeslat opravy
* otevřít Issue s dalšími informacemi
* odeslat Pull Request s vylepšeními

Vzhledem k tomu, že se jedná o projekt zaměřený na výzkum v oblasti ochrany památek a volného času, nemusí být reakce na Issues a Pull Requesty vždy okamžité.

---

## Původní materiál od společnosti Schulze

Toto úložiště rozlišuje mezi:

1. originálním dílem vytvořeným v rámci tohoto projektu reverzního inženýrství a

2. materiálem chráněným autorským právem původně vytvořeným společností Schulze Elektronik GmbH.

Zde mohou být publikovány poznámky k reverznímu inženýrství, originální softwarové nástroje, měření, fotografie, diagramy a nově vytvořená dokumentace.

Originální obrazy firmwaru, manuály a další materiál chráněný autorským právem od společnosti Schulze budou zahrnuty pouze tam, kde je známo, že to práva na redistribuci umožňují.

Pokud práva na redistribuci nejsou jasná, úložiště místo toho materiál zdokumentuje a poskytne informace potřebné pro uživatele, kteří již vlastní původní soubory nebo hardware.

---

## Zabezpečení

Nabíječky baterií a balancery mohou pracovat se značným elektrickým výkonem.

Upravený firmware, kalibrační postupy a experimentální hardware mohou deaktivovat nebo obejít běžné ochranné mechanismy.

Nesprávná kalibrace nebo úprava může vést k nepřesným měřením napětí nebo proudu, poškození baterie, poškození zařízení, požáru nebo jiným nebezpečným podmínkám.

Informace v tomto úložišti jsou poskytovány pro výzkumné, opravárenské a vzdělávací účely. Každý, kdo reprodukuje experimenty nebo úpravy, je zodpovědný za ověření výsledků a použití vhodných bezpečnostních opatření.

---

## Stav projektu

**Práce probíhá.**

Významná část reverzního inženýrství již byla dokončena, zejména pro platformu ISL8. Samostatné projekty ISL8 XTOOLS, integrované projekty ISL8 XTOOLS, ISL6 XTOOLS a ISL6 GUI dosáhly funkčních a hardwarově otestovaných milníků. Další výzkumy a dokumentace stále probíhají.

Tento nezávislý archiv spravuje Martin Stránský (Prometheuss80).

Není přidružen k společnosti Schulze Elektronik, není jí sponzorován ani schválen.

Veškeré ochranné známky a originální názvy produktů zůstávají majetkem příslušných vlastníků.