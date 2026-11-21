# Schulze ISL8 — F2 patch tovární kalibrace

Český stručný návod. Hlavní technická dokumentace je v [README.md](README.md).

**Vydání 1.2: všechny čtyři verze byly otestovány na skutečném ISL8.**

Podporuje originály **8.10 DE, 8.10 EN, 8.50 DE, 8.50 EN**. Skript striktně kontroluje SHA-256 originálního 64kB BIN a nikdy jej nepřepisuje. Neznámé verze odmítne.

```sh
python patch_factory_calibration.py original.bin --dry-run
python patch_factory_calibration.py original.bin
```

Podle verze vznikne `fw819de.bin`, `fw819en.bin`, `fw859de.bin` nebo `fw859en.bin`.

Úprava zpřístupní původní tovární kalibraci pomocí **F2**, přepíše zobrazované označení na **8.19 / 8.59** a textové hodnoty data/času na **22.04.26 / 18:30** (u EN `we`, u DE `Mi`). Nejde o oficiální vydání Schulze ani o nastavení hodin RTC.

**Pozor:** tovární kalibrace je servisní funkce. Před programováním uchovejte zálohu ROM a pokud možno také Timekeeperu. V repozitáři se nedistribuuje žádný originální ani upravený firmware.

Přesné HEX změny: [docs/HEX_CHANGES.md](docs/HEX_CHANGES.md). Vysvětlení principu: [docs/PATCH_TECHNICAL.md](docs/PATCH_TECHNICAL.md).

## Autor a licence

**Martin Stránský (Prometheuss80)** — autor projektu, analýza firmware a ověřování na hardware.

Vlastní Python skript, testy a původní dokumentace projektu jsou zveřejněny pod [licencí MIT](LICENSE). Při dalším šíření je třeba zachovat údaj o autorovi a text licence. Licence se **nevztahuje na originální firmware Schulze**. Projekt není oficiálním produktem Schulze Elektronik.
