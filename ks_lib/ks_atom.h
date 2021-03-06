#ifndef KS_ATOM_H
#define KS_ATON_H

#define KS_ATOM_LIST_NUM 124
typedef struct KS_ATOM_LIST KS_ATOM_LIST;
struct KS_ATOM_LIST{
  char *name;
  char symbol[3];
  char *weight;
  float radius;
  float color[3];
  //  KS_ATOM_LIST *next;
};

#if 1
static KS_ATOM_LIST __KS_USED__ ks_atomic[KS_ATOM_LIST_NUM] = {
/*   0 */ {"",             "",    "",               11.00,{ 0.00, 0.00, 0.00}},
/*   1 */ {"Hydrogen",     "H",   "1.00794(7)",      1.20,{ 1.00, 1.00, 1.00}},
/*   2 */ {"Helium",       "He",  "4.002602(2)",     1.00,{ 1.00, 0.75, 0.80}},
/*   3 */ {"Lithium",      "Li",  "6.941(2)",        1.14,{ 0.70, 0.13, 0.13}},
/*   4 */ {"Beryllium",    "Be",  "9.012182(3)",     1.00,{ 0.76, 1.00, 0.00}},
/*   5 */ {"Boron",        "B",   "10.811(7)",       1.80,{ 0.00, 1.00, 0.00}},
/*   6 */ {"Carbon",       "C",   "12.0107(8)",      1.70,{ 0.78, 0.78, 0.78}},
/*   7 */ {"Nitrogen",     "N",   "14.0067(2)",      1.55,{ 0.56, 0.56, 1.00}},
/*   8 */ {"Oxygen",       "O",   "15.9994(3)",      1.52,{ 0.94, 0.00, 0.00}},
/*   9 */ {"Fluorine",     "F",   "18.9984032(5)",   1.75,{ 0.85, 0.65, 0.13}},
/*  10 */ {"Neon",         "Ne",  "20.1797(6)",      1.40,{ 0.70, 0.89, 0.96}},
/*  11 */ {"Sodium",       "Na",  "22.989770(2)",    1.87,{ 0.00, 0.00, 1.00}},
/*  12 */ {"Magnesium",    "Mg",  "24.3050(6)",      0.79,{ 0.13, 0.55, 0.13}},
/*  13 */ {"Aluminium",    "Al",  "26.981538(2)",    1.90,{ 0.50, 0.50, 0.56}},
/*  14 */ {"Silicon",      "Si",  "28.0855(3)",      1.80,{ 0.85, 0.65, 0.13}},
/*  15 */ {"Phosphorus",   "P",   "30.973761(2)",    2.10,{ 1.00, 0.65, 0.00}},
/*  16 */ {"Sulfur",       "S",   "32.065(5)",       2.00,{ 1.00, 0.78, 0.20}},
/*  17 */ {"Chlorine",     "Cl",  "35.453(2)",       2.47,{ 0.00, 1.00, 0.00}},
/*  18 */ {"Argon",        "Ar",  "39.948(1)",       1.70,{ 0.50, 0.82, 0.89}},
/*  19 */ {"Potassium",    "K",   "39.0983(1)",      2.66,{ 0.56, 0.25, 0.83}},
/*  20 */ {"Calcium",      "Ca",  "40.078(4)",       1.71,{ 0.50, 0.50, 0.56}},
/*  21 */ {"Scandium",     "Sc",  "44.955910(8)",   11.00,{ 0.90, 0.90, 0.90}},
/*  22 */ {"Titanium",     "Ti",  "47.867(1)",      11.00,{ 0.50, 0.50, 0.56}},
/*  23 */ {"Vanadium",     "V",   "50.9415(1)",      0.70,{ 0.65, 0.65, 0.67}},
/*  24 */ {"Chromium",     "Cr",  "51.9961(6)",     11.00,{ 0.50, 0.50, 0.56}},
/*  25 */ {"Manganese",    "Mn",  "54.938049(9)",   11.00,{ 0.50, 0.50, 0.56}},
/*  26 */ {"Iron",         "Fe",  "55.845(2)",       0.63,{ 1.00, 0.65, 0.00}},
/*  27 */ {"Cobalt",       "Co",  "58.933(9)",      11.00,{ 0.94, 0.56, 0.63}},
/*  28 */ {"Nickel",       "Ni",  "58.6934(2)",     11.00,{ 0.65, 0.16, 0.16}},
/*  29 */ {"Copper",       "Cu",  "63.546(3)",       1.00,{ 0.65, 0.16, 0.16}},
/*  30 */ {"Zinc",         "Zn",  "65.409(4)",       1.10,{ 0.65, 0.16, 0.16}},
/*  31 */ {"Gallium",      "Ga",  "69.723(1)",      11.00,{ 0.76, 0.56, 0.56}},
/*  32 */ {"Germanium",    "Ge",  "72.64(1)",       11.00,{ 0.40, 0.56, 0.56}},
/*  33 */ {"Arsenic",      "As",  "74.92160(2)",    11.00,{ 0.74, 0.50, 0.89}},
/*  34 */ {"Selenium",     "Se",  "78.96(3)",       11.00,{ 1.00, 0.63, 0.00}},
/*  35 */ {"Bromine",      "Br",  "79.904(1)",       2.22,{ 0.65, 0.16, 0.16}},
/*  36 */ {"Krypton",      "Kr",  "83.798(2)",      11.00,{ 0.36, 0.72, 0.82}},
/*  37 */ {"Rubidium",     "Rb",  "85.4678(3)",      2.96,{ 0.44, 0.18, 0.69}},
/*  38 */ {"Strontium",    "Sr",  "87.62(1)",       11.00,{ 0.00, 1.00, 0.00}},
/*  39 */ {"Yttrium",      "Y",   "88.90585(2)",    11.00,{ 0.58, 1.00, 1.00}},
/*  40 */ {"Zirconium",    "Zr",  "91.224(2)",      11.00,{ 0.58, 0.88, 0.88}},
/*  41 */ {"Niobium",      "Nb",  "92.906 38(2)",   11.00,{ 0.45, 0.76, 0.79}},
/*  42 */ {"Molybdenum",   "Mo",  "95.94(2)",       11.00,{ 0.33, 0.71, 0.71}},
/*  43 */ {"Technetium",   "Tc",  "[98]",           11.00,{ 0.23, 0.62, 0.62}},
/*  44 */ {"Ruthenium",    "Ru",  "101.07(2)",      11.00,{ 0.14, 0.56, 0.56}},
/*  45 */ {"Rhodium",      "Rh",  "102.90550(2)",   11.00,{ 0.04, 0.49, 0.55}},
/*  46 */ {"Palladium",    "Pd",  "106.42(1)",      11.00,{ 0.00, 0.41, 0.52}},
/*  47 */ {"Silver",       "Ag",  "107.8682(2)",    11.00,{ 0.50, 0.50, 0.56}},
/*  48 */ {"Cadmium",      "Cd",  "112.411(8)",     11.00,{ 1.00, 0.85, 0.56}},
/*  49 */ {"Indium",       "In",  "114.818(3)",     11.00,{ 0.65, 0.46, 0.45}},
/*  50 */ {"Tin",          "Sn",  "118.710(7)",     11.00,{ 0.40, 0.50, 0.50}},
/*  51 */ {"Antimony",     "Sb",  "121.760(1)",     11.00,{ 0.62, 0.39, 0.71}},
/*  52 */ {"Tellurium",    "Te",  "127.60(3)",      11.00,{ 0.83, 0.48, 0.00}},
/*  53 */ {"Iodine",       "I",   "126.90447(3)",    2.35,{ 0.63, 0.13, 0.94}},
/*  54 */ {"Xenon",        "Xe",  "131.293(6)",     11.00,{ 0.26, 0.62, 0.69}},
/*  55 */ {"Caesium",      "Cs",  "132.90545(2)",    3.40,{ 0.34, 0.09, 0.56}},
/*  56 */ {"Barium",       "Ba",  "137.327(7)",     11.00,{ 1.00, 0.65, 0.00}},
/*  57 */ {"Lanthanum",    "La",  "138.9055(2)",    11.00,{ 0.44, 0.83, 1.00}},
/*  58 */ {"Cerium",       "Ce",  "140.116(1)",     11.00,{ 1.00, 1.00, 0.78}},
/*  59 */ {"Praseodymium", "Pr",  "140.90765(2)",   11.00,{ 0.85, 1.00, 0.78}},
/*  60 */ {"Neodymium",    "Nd",  "144.24(3)",      11.00,{ 0.78, 1.00, 0.78}},
/*  61 */ {"Promethium",   "Pm",  "[145]",          11.00,{ 0.64, 1.00, 0.78}},
/*  62 */ {"Samarium",     "Sm",  "150.36(3)",      11.00,{ 0.56, 1.00, 0.78}},
/*  63 */ {"Europium",     "Eu",  "151.964(1)",     11.00,{ 0.38, 1.00, 0.78}},
/*  64 */ {"Gadolinium",   "Gd",  "157.25(3)",      11.00,{ 0.27, 1.00, 0.78}},
/*  65 */ {"Terbium",      "Tb",  "158.92534(2)",   11.00,{ 0.19, 1.00, 0.78}},
/*  66 */ {"Dysprosium",   "Dy",  "162.500(1)",     11.00,{ 0.12, 1.00, 0.78}},
/*  67 */ {"Holmium",      "Ho",  "164.93032(2)",   11.00,{ 0.00, 1.00, 0.61}},
/*  68 */ {"Erbium",       "Er",  "167.259(3)",     11.00,{ 0.00, 0.90, 0.46}},
/*  69 */ {"Thulium",      "Tm",  "168.93421(2)",   11.00,{ 0.00, 0.83, 0.32}},
/*  70 */ {"Ytterbium",    "Yb",  "173.04(3)",      11.00,{ 0.00, 0.75, 0.22}},
/*  71 */ {"Lutetium",     "Lu",  "174.967(1)",     11.00,{ 0.00, 0.67, 0.14}},
/*  72 */ {"Hafnium",      "Hf",  "178.49(2)",      11.00,{ 0.30, 0.76, 1.00}},
/*  73 */ {"Tantalum",     "Ta",  "180.9479(1)",    11.00,{ 0.30, 0.65, 1.00}},
/*  74 */ {"Tungsten",     "W",   "183.84(1)",      11.00,{ 0.13, 0.58, 0.84}},
/*  75 */ {"Rhenium",      "Re",  "186.207(1)",     11.00,{ 0.15, 0.49, 0.67}},
/*  76 */ {"Osmium",       "Os",  "190.23(3)",      11.00,{ 0.15, 0.40, 0.59}},
/*  77 */ {"Iridium",      "Ir",  "192.217(3)",     11.00,{ 0.09, 0.33, 0.53}},
/*  78 */ {"Platinum",     "Pt",  "195.078(2)",     11.00,{ 0.82, 0.82, 0.88}},
/*  79 */ {"Gold",         "Au",  "196.96655(2)",   11.00,{ 0.85, 0.65, 0.13}},
/*  80 */ {"Mercury",      "Hg",  "200.59(2)",      11.00,{ 0.72, 0.72, 0.82}},
/*  81 */ {"Thallium",     "Tl",  "204.3833(2)",    11.00,{ 0.65, 0.33, 0.30}},
/*  82 */ {"Lead",         "Pb",  "207.2(1)",       11.00,{ 0.34, 0.35, 0.38}},
/*  83 */ {"Bismuth",      "Bi",  "208.98038(2)",   11.00,{ 0.62, 0.31, 0.71}},
/*  84 */ {"Polonium",     "Po",  "[210]",          11.00,{ 0.67, 0.36, 0.00}},
/*  85 */ {"Astatine",     "At",  "[210]",          11.00,{ 0.46, 0.31, 0.27}},
/*  86 */ {"Radon",        "Rn",  "[220]",          11.00,{ 0.26, 0.51, 0.59}},
/*  87 */ {"Francium",     "Fr",  "[223]",          11.00,{ 0.26, 0.00, 0.40}},
/*  88 */ {"Radium",       "Ra",  "[226]",          11.00,{ 0.00, 0.49, 0.00}},
/*  89 */ {"Actinium",     "Ac",  "[227]",          11.00,{ 0.44, 0.67, 0.98}},
/*  90 */ {"Thorium",      "Th",  "232.0381(1)",    11.00,{ 0.00, 0.73, 1.00}},
/*  91 */ {"Protactinium", "Pa",  "231.03588(2)",   11.00,{ 0.00, 0.63, 1.00}},
/*  92 */ {"Uranium",      "U",   "238.02891(3)",   11.00,{ 0.00, 0.56, 1.00}},
/*  93 */ {"Neptunium",    "Np",  "[237]",          11.00,{ 0.00, 0.50, 1.00}},
/*  94 */ {"Plutonium",    "Pu",  "[244]",          11.00,{ 0.00, 0.42, 1.00}},
/*  95 */ {"Americium",    "Am",  "[243]",          11.00,{ 0.33, 0.36, 0.95}},
/*  96 */ {"Curium",       "Cm",  "[247]",          11.00,{ 0.47, 0.36, 0.89}},
/*  97 */ {"Berkelium",    "Bk",  "[247]",          11.00,{ 0.54, 0.31, 0.89}},
/*  98 */ {"Californium",  "Cf",  "[251]",          11.00,{ 0.63, 0.21, 0.83}},
/*  99 */ {"Einsteinium",  "Es",  "[252]",          11.00,{ 0.70, 0.12, 0.83}},
/* 100 */ {"Fermium",      "Fm",  "[257]",          11.00,{ 0.70, 0.12, 0.73}},
/* 101 */ {"Mendelevium",  "Md",  "[258]",          11.00,{ 0.70, 0.05, 0.65}},
/* 102 */ {"Nobelium",     "No",  "[259]",          11.00,{ 0.74, 0.05, 0.53}},
/* 103 */ {"Lawrencium",   "Lr",  "[262]",          11.00,{ 0.78, 0.00, 0.40}},
/* 104 */ {"Rutherfordium","Rf",  "261^",           11.00,{ 0.80, 0.00, 0.35}},
/* 105 */ {"Dubnium",      "Db",  "[262]",          11.00,{ 0.82, 0.00, 0.31}},
/* 106 */ {"Seaborgium",   "Sg",  "[266]",          11.00,{ 0.85, 0.00, 0.27}},
/* 107 */ {"Bohrium",      "Bh",  "[264]",          11.00,{ 0.88, 0.00, 0.22}},
/* 108 */ {"Hassium",      "Hs",  "[277]",          11.00,{ 0.90, 0.00, 0.18}},
/* 109 */ {"Meitnerium",   "Mt",  "[268]",          11.00,{ 0.00, 0.00, 0.00}},
/* 110 */ {"Darmstadtium", "Ds",  "[271]",          11.00,{ 0.00, 0.00, 0.00}},
/* 111 */ {"Roentgenium",  "Rg",  "[272]",          11.00,{ 0.00, 0.00, 0.00}},
/* 112 */ {"",             "",    "",               11.00,{ 1.00, 0.00, 0.00}},
/* 113 */ {"",             "",    "",               11.00,{ 0.00, 1.00, 0.00}},
/* 114 */ {"",             "",    "",               11.00,{ 0.00, 0.00, 1.00}},
/* 115 */ {"",             "",    "",               11.00,{ 0.00, 1.00, 1.00}},
/* 116 */ {"",             "",    "",               11.00,{ 1.00, 0.00, 1.00}},
/* 117 */ {"",             "",    "",               11.00,{ 1.00, 1.00, 0.00}},
/* 118 */ {"",             "",    "",               11.00,{ 0.00, 0.00, 0.00}},
/* 119 */ {"",             "",    "",               11.00,{ 0.00, 0.00, 0.00}},
/* 120 */ {"LJ",           "LJ",  "12.0107(8)",      1.70,{ 0.60, 0.60, 0.60}},
/* 121 */ {"Lone Pair",    "LP",  "",                0.10,{ 1.00, 0.08, 0.94}},
/* 122 */ {"DUMM",         "DU",  "",                0.50,{ 0.40, 0.40, 0.40}},
/* 123 */ {"END",          "",    "",               11.00,{ 0.00, 0.00, 0.00}},
};
#else
static KS_ATOM_LIST ks_atomic[KS_ATOM_LIST_NUM] = {
  /*   0 */ {"",             "",    "",               11.00,{ 0.00, 0.00, 0.00}},
  /*   1 */ {"Hydrogen",     "H",   "1.00794(7)",      1.20,{ 1.00, 1.00, 1.00}},
  /*   2 */ {"Helium",       "He",  "4.002602(2)",    11.00,{ 0.00, 0.00, 0.00}},
  /*   3 */ {"Lithium",      "Li",  "6.941(2)",        1.14,{ 0.70, 0.13, 0.13}},
  /*   4 */ {"Beryllium",    "Be",  "9.012182(3)",    11.00,{ 0.00, 0.00, 0.00}},
  /*   5 */ {"Boron",        "B",   "10.811(7)",      11.00,{ 0.00, 0.00, 0.00}},
  /*   6 */ {"Carbon",       "C",   "12.0107(8)",      1.70,{ 0.78, 0.78, 0.78}},
  /*   7 */ {"Nitrogen",     "N",   "14.0067(2)",      1.55,{ 0.56, 0.56, 1.00}},
  /*   8 */ {"Oxygen",       "O",   "15.9994(3)",      1.52,{ 0.94, 0.00, 0.00}},
  /*   9 */ {"Fluorine",     "F",   "18.9984032(5)",   1.75,{ 0.70, 1.00, 0.70}},
  /*  10 */ {"Neon",         "Ne",  "20.1797(6)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  11 */ {"Sodium",       "Na",  "22.989770(2)",    1.87,{ 0.00, 0.00, 1.00}},
  /*  12 */ {"Magnesium",    "Mg",  "24.3050(6)",      0.79,{ 1.00, 0.08, 0.58}},
  /*  13 */ {"Aluminium",    "Al",  "26.981538(2)",   11.00,{ 0.00, 0.00, 0.00}},
  /*  14 */ {"Silicon",      "Si",  "28.0855(3)",      1.80,{ 1.00, 0.78, 0.20}},
  /*  15 */ {"Phosphorus",   "P",   "30.973761(2)",    2.10,{ 1.00, 0.65, 0.00}},
  /*  16 */ {"Sulfur",       "S",   "32.065(5)",       2.00,{ 1.00, 0.78, 0.20}},
  /*  17 */ {"Chlorine",     "Cl",  "35.453(2)",       2.47,{ 0.00, 1.00, 0.00}},
  /*  18 */ {"Argon",        "Ar",  "39.948(1)",       1.70,{ 0.70, 0.70, 0.70}},
  /*  19 */ {"Potassium",    "K",   "39.0983(1)",      2.66,{ 0.20, 0.20, 0.20}},
  /*  20 */ {"Calcium",      "Ca",  "40.078(4)",       1.71,{ 0.50, 0.50, 0.50}},
  /*  21 */ {"Scandium",     "Sc",  "44.955910(8)",   11.00,{ 0.00, 0.00, 0.00}},
  /*  22 */ {"Titanium",     "Ti",  "47.867(1)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  23 */ {"Vanadium",     "V",   "50.9415(1)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  24 */ {"Chromium",     "Cr",  "51.9961(6)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  25 */ {"Manganese",    "Mn",  "54.938049(9)",   11.00,{ 0.00, 0.00, 0.00}},
  /*  26 */ {"Iron",         "Fe",  "55.845(2)",       0.63,{ 1.00, 0.65, 0.00}},
  /*  27 */ {"Cobalt",       "Co",  "58.933(9)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  28 */ {"Nickel",       "Ni",  "58.6934(2)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  29 */ {"Copper",       "Cu",  "63.546(3)",       1.00,{ 0.65, 0.16, 0.16}},
  /*  30 */ {"Zinc",         "Zn",  "65.409(4)",       1.10,{ 0.65, 0.16, 0.16}},
  /*  31 */ {"Gallium",      "Ga",  "69.723(1)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  32 */ {"Germanium",    "Ge",  "72.64(1)",       11.00,{ 0.00, 0.00, 0.00}},
  /*  33 */ {"Arsenic",      "As",  "74.92160(2)",    11.00,{ 0.00, 0.00, 0.00}},
  /*  34 */ {"Selenium",     "Se",  "78.96(3)",       11.00,{ 0.00, 0.00, 0.00}},
  /*  35 */ {"Bromine",      "Br",  "79.904(1)",       2.22,{ 0.65, 0.16, 0.16}},
  /*  36 */ {"Krypton",      "Kr",  "83.798(2)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  37 */ {"Rubidium",     "Rb",  "85.4678(3)",      2.96,{ 0.20, 0.20, 0.20}},
  /*  38 */ {"Strontium",    "Sr",  "87.62(1)",       11.00,{ 0.00, 0.00, 0.00}},
  /*  39 */ {"Yttrium",      "Y",   "88.90585(2)",    11.00,{ 0.00, 0.00, 0.00}},
  /*  40 */ {"Zirconium",    "Zr",  "91.224(2)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  41 */ {"Niobium",      "Nb",  "92.906 38(2)",   11.00,{ 0.00, 0.00, 0.00}},
  /*  42 */ {"Molybdenum",   "Mo",  "95.94(2)",       11.00,{ 0.00, 0.00, 0.00}},
  /*  43 */ {"Technetium",   "Tc",  "[98]",           11.00,{ 0.00, 0.00, 0.00}},
  /*  44 */ {"Ruthenium",    "Ru",  "101.07(2)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  45 */ {"Rhodium",      "Rh",  "102.90550(2)",   11.00,{ 0.00, 0.00, 0.00}},
  /*  46 */ {"Palladium",    "Pd",  "106.42(1)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  47 */ {"Silver",       "Ag",  "107.8682(2)",    11.00,{ 0.00, 0.00, 0.00}},
  /*  48 */ {"Cadmium",      "Cd",  "112.411(8)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  49 */ {"Indium",       "In",  "114.818(3)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  50 */ {"Tin",          "Sn",  "118.710(7)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  51 */ {"Antimony",     "Sb",  "121.760(1)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  52 */ {"Tellurium",    "Te",  "127.60(3)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  53 */ {"Iodine",       "I",   "126.90447(3)",    2.35,{ 0.63, 0.13, 0.94}},
  /*  54 */ {"Xenon",        "Xe",  "131.293(6)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  55 */ {"Caesium",      "Cs",  "132.90545(2)",    3.40,{ 0.20, 0.20, 0.20}},
  /*  56 */ {"Barium",       "Ba",  "137.327(7)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  57 */ {"Lanthanum",    "La",  "138.9055(2)",    11.00,{ 0.00, 0.00, 0.00}},
  /*  58 */ {"Cerium",       "Ce",  "140.116(1)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  59 */ {"Praseodymium", "Pr",  "140.90765(2)",   11.00,{ 0.00, 0.00, 0.00}},
  /*  60 */ {"Neodymium",    "Nd",  "144.24(3)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  61 */ {"Promethium",   "Pm",  "[145]",          11.00,{ 0.00, 0.00, 0.00}},
  /*  62 */ {"Samarium",     "Sm",  "150.36(3)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  63 */ {"Europium",     "Eu",  "151.964(1)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  64 */ {"Gadolinium",   "Gd",  "157.25(3)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  65 */ {"Terbium",      "Tb",  "158.92534(2)",   11.00,{ 0.00, 0.00, 0.00}},
  /*  66 */ {"Dysprosium",   "Dy",  "162.500(1)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  67 */ {"Holmium",      "Ho",  "164.93032(2)",   11.00,{ 0.00, 0.00, 0.00}},
  /*  68 */ {"Erbium",       "Er",  "167.259(3)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  69 */ {"Thulium",      "Tm",  "168.93421(2)",   11.00,{ 0.00, 0.00, 0.00}},
  /*  70 */ {"Ytterbium",    "Yb",  "173.04(3)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  71 */ {"Lutetium",     "Lu",  "174.967(1)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  72 */ {"Hafnium",      "Hf",  "178.49(2)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  73 */ {"Tantalum",     "Ta",  "180.9479(1)",    11.00,{ 0.00, 0.00, 0.00}},
  /*  74 */ {"Tungsten",     "W",   "183.84(1)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  75 */ {"Rhenium",      "Re",  "186.207(1)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  76 */ {"Osmium",       "Os",  "190.23(3)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  77 */ {"Iridium",      "Ir",  "192.217(3)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  78 */ {"Platinum",     "Pt",  "195.078(2)",     11.00,{ 0.00, 0.00, 0.00}},
  /*  79 */ {"Gold",         "Au",  "196.96655(2)",   11.00,{ 0.00, 0.00, 0.00}},
  /*  80 */ {"Mercury",      "Hg",  "200.59(2)",      11.00,{ 0.00, 0.00, 0.00}},
  /*  81 */ {"Thallium",     "Tl",  "204.3833(2)",    11.00,{ 0.00, 0.00, 0.00}},
  /*  82 */ {"Lead",         "Pb",  "207.2(1)",       11.00,{ 0.00, 0.00, 0.00}},
  /*  83 */ {"Bismuth",      "Bi",  "208.98038(2)",   11.00,{ 0.00, 0.00, 0.00}},
  /*  84 */ {"Polonium",     "Po",  "[210]",          11.00,{ 0.00, 0.00, 0.00}},
  /*  85 */ {"Astatine",     "At",  "[210]",          11.00,{ 0.00, 0.00, 0.00}},
  /*  86 */ {"Radon",        "Rn",  "[220]",          11.00,{ 0.00, 0.00, 0.00}},
  /*  87 */ {"Francium",     "Fr",  "[223]",          11.00,{ 0.00, 0.00, 0.00}},
  /*  88 */ {"Radium",       "Ra",  "[226]",          11.00,{ 0.00, 0.00, 0.00}},
  /*  89 */ {"Actinium",     "Ac",  "[227]",          11.00,{ 0.00, 0.00, 0.00}},
  /*  90 */ {"Thorium",      "Th",  "232.0381(1)",    11.00,{ 0.00, 0.00, 0.00}},
  /*  91 */ {"Protactinium", "Pa",  "231.03588(2)",   11.00,{ 0.00, 0.00, 0.00}},
  /*  92 */ {"Uranium",      "U",   "238.02891(3)",   11.00,{ 0.00, 0.00, 0.00}},
  /*  93 */ {"Neptunium",    "Np",  "[237]",          11.00,{ 0.00, 0.00, 0.00}},
  /*  94 */ {"Plutonium",    "Pu",  "[244]",          11.00,{ 0.00, 0.00, 0.00}},
  /*  95 */ {"Americium",    "Am",  "[243]",          11.00,{ 0.00, 0.00, 0.00}},
  /*  96 */ {"Curium",       "Cm",  "[247]",          11.00,{ 0.00, 0.00, 0.00}},
  /*  97 */ {"Berkelium",    "Bk",  "[247]",          11.00,{ 0.00, 0.00, 0.00}},
  /*  98 */ {"Californium",  "Cf",  "[251]",          11.00,{ 0.00, 0.00, 0.00}},
  /*  99 */ {"Einsteinium",  "Es",  "[252]",          11.00,{ 0.00, 0.00, 0.00}},
  /* 100 */ {"Fermium",      "Fm",  "[257]",          11.00,{ 0.00, 0.00, 0.00}},
  /* 101 */ {"Mendelevium",  "Md",  "[258]",          11.00,{ 0.00, 0.00, 0.00}},
  /* 102 */ {"Nobelium",     "No",  "[259]",          11.00,{ 0.00, 0.00, 0.00}},
  /* 103 */ {"Lawrencium",   "Lr",  "[262]",          11.00,{ 0.00, 0.00, 0.00}},
  /* 104 */ {"Rutherfordium","Rf",  "261^",           11.00,{ 0.00, 0.00, 0.00}},
  /* 105 */ {"Dubnium",      "Db",  "[262]",          11.00,{ 0.00, 0.00, 0.00}},
  /* 106 */ {"Seaborgium",   "Sg",  "[266]",          11.00,{ 0.00, 0.00, 0.00}},
  /* 107 */ {"Bohrium",      "Bh",  "[264]",          11.00,{ 0.00, 0.00, 0.00}},
  /* 108 */ {"Hassium",      "Hs",  "[277]",          11.00,{ 0.00, 0.00, 0.00}},
  /* 109 */ {"Meitnerium",   "Mt",  "[268]",          11.00,{ 0.00, 0.00, 0.00}},
  /* 110 */ {"Darmstadtium", "Ds",  "[271]",          11.00,{ 0.00, 0.00, 0.00}},
  /* 111 */ {"Roentgenium",  "Rg",  "[272]",          11.00,{ 0.00, 0.00, 0.00}},
  /* 112 */ {"",             "",    "",               11.00,{ 0.00, 0.00, 0.00}},
  /* 113 */ {"",             "",    "",               11.00,{ 0.00, 0.00, 0.00}},
  /* 114 */ {"",             "",    "",               11.00,{ 0.00, 0.00, 0.00}},
  /* 115 */ {"",             "",    "",               11.00,{ 0.00, 0.00, 0.00}},
  /* 116 */ {"",             "",    "",               11.00,{ 0.00, 0.00, 0.00}},
  /* 117 */ {"",             "",    "",               11.00,{ 0.00, 0.00, 0.00}},
  /* 118 */ {"",             "",    "",               11.00,{ 0.00, 0.00, 0.00}},
  /* 119 */ {"",             "",    "",               11.00,{ 0.00, 0.00, 0.00}},
  /* 120 */ {"",             "",    "",               11.00,{ 0.00, 0.00, 0.00}},
  /* 121 */ {"Lone Pair",    "LP",  "",                0.10,{ 1.00, 0.08, 0.94}},
  /* 122 */ {"DUMM",         "DU",  "",                0.50,{ 0.40, 0.40, 0.40}},
  /* 123 */ {"END",          "",    "",               11.00,{ 0.00, 0.00, 0.00}},
};
#endif

#define KS_AMINO_LIST_NUM 20
enum {
  KS_AMINO_ACIDIC    =  3, /* GLU color */
  KS_AMINO_BASIC     = 14, /* ARG color */
  KS_AMINO_POLAR     = 17, /* VAL color */
  KS_AMINO_NO_CHARGE =  4, /* PHE color */
  KS_AMINO_TYPE_NUM = 4
};
typedef struct KS_AMINO_LIST KS_AMINO_LIST;
struct KS_AMINO_LIST{
  char *name;
  char c;
  float color[3];
  double hydropathy;
  int type;
};

extern KS_AMINO_LIST ks_amino[];

static __KS_USED__ struct {
  char symbol[3];
  char Jmol_color[7];
  char Rasmol_color[7];
} _atom_colors[] = {
  /* 	 	Jmol	 	Rasmol */
  {"H ",	"FFFFFF",	"FFFFFF"}, /*     1  */
  {"He",	"D9FFFF",	"FFC0CB"}, /*	  2  */
  {"Li",	"CC80FF",	"B22222"}, /* 	  3  */
  {"Be",	"C2FF00",	"FF1493"}, /*	  4  */
  {"B ",	"FFB5B5",	"00FF00"}, /*	  5  */
  {"C ",	"909090",	"C8C8C8"}, /* 	  6  */
  {"N ",	"3050F8",	"8F8FFF"}, /* 	  7  */
  {"O ",	"FF0D0D",	"F00000"}, /* 	  8  */
  {"F ",	"90E050",	"DAA520"}, /*	  9  */
  {"Ne",	"B3E3F5",	"FF1493"}, /*	 10  */
  {"Na",	"AB5CF2",	"0000FF"}, /*	 11  */
  {"Mg",	"8AFF00",	"228B22"}, /*	 12  */
  {"Al",	"BFA6A6",	"808090"}, /* 	 13  */
  {"Si",	"F0C8A0",	"DAA520"}, /*	 14  */
  {"P ",	"FF8000",	"FFA500"}, /* 	 15  */
  {"S ",	"FFFF30",	"FFC832"}, /* 	 16  */
  {"Cl",	"1FF01F",	"00FF00"}, /*	 17  */
  {"Ar",	"80D1E3",	"FF1493"}, /*	 18  */
  {"K ",	"8F40D4",	"FF1493"}, /*	 19  */
  {"Ca",	"3DFF00",	"808090"}, /* 	 20  */
  {"Sc",	"E6E6E6",	"FF1493"}, /*	 21  */
  {"Ti",	"BFC2C7",	"808090"}, /* 	 22  */
  {"V ",	"A6A6AB",	"FF1493"}, /*	 23  */
  {"Cr",	"8A99C7",	"808090"}, /* 	 24  */
  {"Mn",	"9C7AC7",	"808090"}, /* 	 25  */
  {"Fe",	"E06633",	"FFA500"}, /* 	 26  */
  {"Co",	"F090A0",	"FF1493"}, /*	 27  */
  {"Ni",	"50D050",	"A52A2A"}, /* 	 28  */
  {"Cu",	"C88033",	"A52A2A"}, /* 	 29  */
  {"Zn",	"7D80B0",	"A52A2A"}, /* 	 30  */
  {"Ga",	"C28F8F",	"FF1493"}, /*	 31  */
  {"Ge",	"668F8F",	"FF1493"}, /*	 32  */
  {"As",	"BD80E3",	"FF1493"}, /*	 33  */
  {"Se",	"FFA100",	"FF1493"}, /*	 34  */
  {"Br",	"A62929",	"A52A2A"}, /* 	 35  */
  {"Kr",	"5CB8D1",	"FF1493"}, /*	 36  */
  {"Rb",	"702EB0",	"FF1493"}, /*	 37  */
  {"Sr",	"00FF00",	"FF1493"}, /*	 38  */
  {"Y ",	"94FFFF",	"FF1493"}, /*	 39  */
  {"Zr",	"94E0E0",	"FF1493"}, /*	 40  */
  {"Nb",	"73C2C9",	"FF1493"}, /*	 41  */
  {"Mo",	"54B5B5",	"FF1493"}, /*	 42  */
  {"Tc",	"3B9E9E",	"FF1493"}, /*	 43  */
  {"Ru",	"248F8F",	"FF1493"}, /*	 44  */
  {"Rh",	"0A7D8C",	"FF1493"}, /*	 45  */
  {"Pd",	"006985",	"FF1493"}, /*	 46  */
  {"Ag",	"C0C0C0",	"808090"}, /* 	 47  */
  {"Cd",	"FFD98F",	"FF1493"}, /*	 48  */
  {"In",	"A67573",	"FF1493"}, /*	 49  */
  {"Sn",	"668080",	"FF1493"}, /*	 50  */
  {"Sb",	"9E63B5",	"FF1493"}, /*	 51  */
  {"Te",	"D47A00",	"FF1493"}, /*	 52  */
  {"I ",	"940094",	"A020F0"}, /*	 53  */
  {"Xe",	"429EB0",	"FF1493"}, /*	 54  */
  {"Cs",	"57178F",	"FF1493"}, /*	 55  */
  {"Ba",	"00C900",	"FFA500"}, /* 	 56  */
  {"La",	"70D4FF",	"FF1493"}, /*	 57  */
  {"Ce",	"FFFFC7",	"FF1493"}, /*	 58  */
  {"Pr",	"D9FFC7",	"FF1493"}, /*	 59  */
  {"Nd",	"C7FFC7",	"FF1493"}, /*	 60  */
  {"Pm",	"A3FFC7",	"FF1493"}, /*	 61  */
  {"Sm",	"8FFFC7",	"FF1493"}, /*	 62  */
  {"Eu",	"61FFC7",	"FF1493"}, /*	 63  */
  {"Gd",	"45FFC7",	"FF1493"}, /*	 64  */
  {"Tb",	"30FFC7",	"FF1493"}, /*	 65  */
  {"Dy",	"1FFFC7",	"FF1493"}, /*	 66  */
  {"Ho",	"00FF9C",	"FF1493"}, /*	 67  */
  {"Er",	"00E675",	"FF1493"}, /*	 68  */
  {"Tm",	"00D452",	"FF1493"}, /*	 69  */
  {"Yb",	"00BF38",	"FF1493"}, /*	 70  */
  {"Lu",	"00AB24",	"FF1493"}, /*	 71  */
  {"Hf",	"4DC2FF",	"FF1493"}, /*	 72  */
  {"Ta",	"4DA6FF",	"FF1493"}, /*	 73  */
  {"W ",	"2194D6",	"FF1493"}, /*	 74  */
  {"Re",	"267DAB",	"FF1493"}, /*	 75  */
  {"Os",	"266696",	"FF1493"}, /*	 76  */
  {"Ir",	"175487",	"FF1493"}, /*	 77  */
  {"Pt",	"D0D0E0",	"FF1493"}, /*	 78  */
  {"Au",	"FFD123",	"DAA520"}, /*	 79  */
  {"Hg",	"B8B8D0",	"FF1493"}, /*	 80  */
  {"Tl",	"A6544D",	"FF1493"}, /*	 81  */
  {"Pb",	"575961",	"FF1493"}, /*	 82  */
  {"Bi",	"9E4FB5",	"FF1493"}, /*	 83  */
  {"Po",	"AB5C00",	"FF1493"}, /*	 84  */
  {"At",	"754F45",	"FF1493"}, /*	 85  */
  {"Rn",	"428296",	"FF1493"}, /*	 86  */
  {"Fr",	"420066",	"FF1493"}, /*	 87  */
  {"Ra",	"007D00",	"FF1493"}, /*	 88  */
  {"Ac",	"70ABFA",	"FF1493"}, /*	 89  */
  {"Th",	"00BAFF",	"FF1493"}, /*	 90  */
  {"Pa",	"00A1FF",	"FF1493"}, /*	 91  */
  {"U ",	"008FFF",	"FF1493"}, /*	 92  */
  {"Np",	"0080FF",	"FF1493"}, /*	 93  */
  {"Pu",	"006BFF",	"FF1493"}, /*	 94  */
  {"Am",	"545CF2",	"FF1493"}, /*	 95  */
  {"Cm",	"785CE3",	"FF1493"}, /*	 96  */
  {"Bk",	"8A4FE3",	"FF1493"}, /*	 97  */
  {"Cf",	"A136D4",	"FF1493"}, /*	 98  */
  {"Es",	"B31FD4",	"FF1493"}, /*	 99  */
  {"Fm",	"B31FBA",	"FF1493"}, /*	100  */
  {"Md",	"B30DA6",	"FF1493"}, /*	101  */
  {"No",	"BD0D87",	"FF1493"}, /*	102  */
  {"Lr",	"C70066",	"FF1493"}, /*	103  */
  {"Rf",	"CC0059",	"FF1493"}, /*	104  */
  {"Db",	"D1004F",	"FF1493"}, /*	105  */
  {"Sg",	"D90045",	"FF1493"}, /*	106  */
  {"Bh",	"E00038",	"FF1493"}, /*	107  */
  {"Hs",	"E6002E",	"FF1493"}, /*	108  */
  {"Mt",	"EB0026",	"FF1493"}  /*	109  */
};

/*
  GLfloat amino_color[21][3] = {
  {0.7 ,1.0 ,0.7 },{0.8 ,0.7 ,0.4 },
  {0.8 ,0.0 ,0.6 },{1.0 ,0.0 ,0.0 },
  {0.8 ,0.8 ,0.8 },{1.0 ,1.0 ,1.0 },
  {0.4 ,0.35,0.8 },{0.0 ,0.6 ,0.0 },
  {0.55,0.41,0.88},{0.7 ,0.7 ,0.7 },
  {0.9 ,0.6 ,0.6 },{0.98,0.5 ,0.45},
  {0.6 ,0.6 ,0.6 },{1.0 ,0.08,0.58},
  {0.0 ,0.0 ,0.5 },{1.0 ,0.39,0.28},
  {1.0 ,0.27,0.0 },{1.0 ,0.6 ,0.9 },
  {1.0 ,1.0 ,0.0 },{0.5 ,0.5 ,0.5 },
  {0.0 ,0.5 ,1.0 }};
*/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  BOOL ks_get_atom_number(char *s, int *n);
  BOOL ks_get_atom_radius(char *s, float *r);
  BOOL ks_get_atom_color(char *s, float *color);
  BOOL ks_get_atom_weight(char *s, double *w);
  /*
  int ks_get_atomic_number(char *s);
  double ks_get_atomic_weight(char *s);
  float ks_get_atomic_radius(char *s);
  float *ks_get_atomic_color(char *s);
  */

  BOOL ks_get_amino_id(char *s, int *id);
  BOOL ks_get_amino_color(char *s, float color[3]);
  void ks_set_amino_color(char *s, float color[3]);
  BOOL ks_get_amino_hydropathy_index(char *s, double *h);
  BOOL ks_get_amino_type(char *s, int *type);
  BOOL ks_get_amino_name_to_char(char *amino_name, char *amino_char);
  BOOL ks_get_amino_char_to_name(char amino_char, char *amino_name);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
