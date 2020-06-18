**What does it do?**

Translates an item script into an item description.

```
{
    bonus bAtkEle,Ele_Dark;
    bonus2 bAddEle,Ele_Ghost,15;
    bonus3 bAutoSpell,"MG_STONECURSE",3,100;
    bonus2 bAddEff,Eff_Stone,10;
    bonus bDex,3;
}
```

```
- id: 1138
  name: Mysteltainn
  bonus: |
    Weapon endowed with Dark property.
    +15% Physical Damage vs. Ghost enemies.
    Add 10% chance to cast Level 3 Stone Curse for each weapon attack.
    Add 0.10% chance to inflict Stone on enemy for each weapon attack.
    DEX +3
```

**How to build?**

```make```

OR

```make CFLAGS=-O2```

**How to use?**

```./pj59 . > output.yml```

```./pj59 . 1138```

**How to setup?**

Copy these files from rAthena to pj59.

* skill_db.yml
* item_combo_db.txt
* item_db.txt
* mercenary_db.txt
* mob_db.txt

**What do you support?**

English and rAthena in Renewal mode.

**What about another language?**

Translate these files and save in UTF-8.

 * statement.yml
 * argument.yml
 * bonus.yml
 * bonus2.yml
 * bonus3.yml
 * bonus4.yml
 * bonus5.yml
 * constant_data.yml
 * sc_start.yml
 * sc_start2.yml
 * sc_start4.yml
