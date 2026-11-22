# Kontrola publikační verze

- Odstraněny složky `Objects/` a `Listings/` včetně originálních obrazů firmwaru a přeložených výstupů.
- Odstraněny soubory uživatelského nastavení Keil `*.uvopt`, `*.uvgui.*`.
- Odstraněn původní `REG80C537_ISL8.h` (copyright Keil). Odkazy změněny na `<REG537.H>` dodaný lokálně nástroji Keil.
- Odstraněn původní Keil `STARTUP.A51` (copyright Keil) a jeho reference v `.uvproj`. Konfiguraci startupu je nutné před sestavením vyřešit z lokální instalace Keil.
- Původní logo převzaté z ROM odstraněno; `lcd_bitmap.c` kreslí nový vlastní symbol.
- Zachovány názvy všech zbývajících zdrojových souborů a původní projekt `XTOOLS_ISL8_SERVICE.uvproj`.

## Zbývající ověření před zveřejněním

1. Zkušební překlad v Keil C51, včetně podpory `REG537.H` a startupu.
2. Porovnání grafického symbolu a obrazovek na obou displejích.
3. Kontrola, zda žádný ze zbývajících zdrojů není od třetí strany; především další asm moduly a kód převzatý z příkladů.
4. Stanovení licence vlastní tvorby a kontrola souborů před finálním commitem.
5. Bezpečnostní testy výkonových režimů na laboratorním pracovišti.

Dual LCD target configuration and custom xtools_boot.a51 added. Build and functional validation pending.
