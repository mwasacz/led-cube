# Led Cube

# Temat projektu

Wyświetlacze typu Led Cube to ażurowe konstrukcje zbudowane z diod LED ułożonych w formie sześcianu. Są one umocowane za pomocą drutów, przez co widoczna jest cała siatka diod. Sterując indywidualnie każdą z nich możliwe jest wyświetlanie prostych animacji.

Celem projektu jest zbudowanie działającej kostki LED 4x4x4 przy użyciu minimalnej liczby komponentów oraz zaprezentowanie niesamowitych możliwości najmniejszych mikrokontrolerów, takich jak użyty w tym projekcie AVR ATtiny85.

# Opis działania urządzenia

![](Photos/Led%20Cube%204.jpg)

Wyświetlacz Led Cube wymaga do działania karty SD z zapisanymi na niej animacjami. Po włożeniu karty i podpięciu zasilania rozpoczyna się prezentacja animacji.

Wyświetlacz może działać w dwóch trybach:

- Tryb automatyczny – animacje są odtwarzane po kolei (po wyświetleniu wszystkich sekwencja powtarza się)
- Tryb ręczny – odtwarzana jest ciągle ta sama animacja

Domyślny jest tryb automatyczny. Po naciśnięciu przycisku kostka przechodzi w tryb ręczny, wyświetla numer 1, a następnie pokazuje ciągle pierwszą animację. Kolejne naciśnięcia powodują zmianę animacji na następną, co sygnalizowane jest wyświetleniem jej numeru. Gdy nie ma następnej animacji, kostka wyświetla napis All i wraca w tryb automatyczny.

W przypadku braku karty SD bądź innych błędów, kostka wyświetla literę E i kod błędu. Możliwe kody przedstawione są w tab. 1 wraz z ich opisem. Po likwidacji źródła problemu należy nacisnąć przycisk, urządzenie uruchomi się ponownie.

| **Kod błędu** | **Nazwa** | **Opis** |
| --- | --- | --- |
| E1 | FR\_DISK\_ERR | Błąd wewnętrzny karty |
| E2 | FR\_NOT\_READY | Brak podpiętej karty / Nie udało się uruchomić karty |
| E3 | FR\_NO\_FILE | Nie znaleziono pliku z animacją |
| E4 | FR\_NOT\_OPENED | Błąd wewnętrzny programu |
| E5 | FR\_NOT\_ENABLED | Błąd wewnętrzny programu |
| E6 | FR\_NO\_FILESYSTEM | Nie znaleziono systemu plików |
| E7 | FILE\_ERR | Błąd w strukturze pliku z animacją |
| E8 | TIMING\_ERR | Plik z animacją zawiera zbyt szybko zmieniające się klatki |

**Tab. 1. Kody błędów**

# Szczegóły projektu

## Budowa urządzenia

Najważniejsze komponenty w układzie elektronicznym wyświetlacza to: mikrokontroler, karta SD, scalony sterownik diod LED i regulator napięcia. Obwód umieszczony jest w plastikowej obudowie, która pełni również funkcję podstawy dla konstrukcji z diod.

### Mikrokontroler

Do sterowania układem został wybrany mikrokontroler AVR ATtiny85. Jest to jeden z mniejszych procesorów rodziny AVR ATtiny, jednak nawet tak niewielki mikrokontroler ma duży potencjał, posiada wystarczającą ilość zasobów, aby obsłużyć ten projekt.

Najważniejsze parametry mikrokontrolera AVR ATtiny85:

- 8-bitowa architektura RISC
- 6 pinów wejścia-wyjścia (z czego jeden przeznaczony na reset)
- 8 kilobajtów pamięci FLASH
- 512 bajtów pamięci EEPROM
- 512 bajtów pamięci RAM
- Napięcie zasilania: 2,7 – 5,5 V
- Taktowanie: do 20 MHz

**Rys. 1. Połączenie interfejsu SPI**

Mikrokontroler komunikuje się z kartą SD oraz sterownikiem diod za pomocą interfejsu SPI. W celu zaoszczędzenia pinów mikrokontrolera, linie interfejsu są współdzielone. Ogólny schemat ich połączenia jest ukazany na rys. 1. Mikrokontroler steruje liniami Clock i Enable. Układ sterownika ma wewnątrz rejestr przesuwny, którego wejściem steruje procesor, a wyjście jest podłączone do wejścia danych karty SD. Wyjście danych z karty jest podpięte do wejścia mikrokontrolera. W ten sposób można sterować kartą i sterownikiem za pomocą czterech pinów procesora. Piąty pin jest używany do obsługi przycisku.

Mikrokontroler zasilany jest napięciem 3 V pochodzącym ze stabilizatora. Taktowany jest wewnętrznym oscylatorem 8 MHz. Podsumowanie funkcji poszczególnych pinów wejścia-wyjścia znajduje się w tab. 2.

| **Pin** | **Kierunek** | **Funkcja** |
| --- | --- | --- |
| PB0 | Wejście | DI – wejście danych interfejsu SPI od karty SD |
| PB1 | Wyjście | DO – wyjście danych interfejsu SPI do sterownika diod |
| PB2 | Wyjście | USCK – wyjście zegarowe interfejsu SPI |
| PB3 | Wyjście | Wyjście podłączone do sterownika oraz karty SD pozwalające na wykonanie komendy |
| PB4 | Wejście | Wejście przycisku do zmiany animacji |

**Tab. 2. Funkcje pinów mikrokontrolera**

Wykorzystane układy peryferyjne mikrokontrolera:

- Universal Serial Interface (USI) – wykorzystywany w trybie SPI do komunikacji z kartą SD i sterownikiem diod LED
- Timer0 – używany do odmierzania czasu trwania klatki animacji
- Pin Change Interrupt – używany do wykrycia naciśnięcia przycisku
- Timer1 – używany do generowania opóźnienia w celu eliminacji drgań styków przycisku

### Sterownik diod LED

Jako sterownik diod LED użyty jest układ scalony MAX7219. Pozwala on sterować 64 diodami połączonymi w matrycę 8x8. Aby dostosować układ diod LED do tego sterownika zastosowano nietypowe połączenie elektryczne. Najczęściej w konstrukcjach typu Led Cube anody jednej poziomej warstwy są połączone razem i katody jednego pionowego rzędu są połączone razem tworząc układ analogiczny do matrycy 4x16. W tym projekcie diody są połączone w inny sposób tworząc połączenie analogiczne do matrycy 8x8. Dzięki temu pojedynczy układ MAX7219 może sterować całą kostką.

Układ komunikuje się z mikrokontrolerem przez interfejs SPI.

### Karta SD

Karta SD ma możliwość komunikacji przez kilka interfejsów, z czego jeden to SPI. Ten właśnie interfejs został użyty do komunikacji z procesorem.

Karta używa systemu plików FAT. Pliki z animacjami nie mają dużego rozmiaru, dlatego karta może być jednocześnie używana do przechowywania innych danych.

### Zasilanie

Układ może być zasilany napięciem od 4,5 do 5,5 V. Takiego właśnie napięcia wymaga sterownik MAX7219. Karta SD powinna być zasilana napięciem od 2,7 do 3,6 V, dlatego zastosowany został stabilizator LP2950 na napięcie 3 V. Zasila on kartę i mikrokontroler. Obwód zawiera również niezbędne kondensatory filtrujące.

Ponieważ układ MAX7219 jest zasilany innym napięciem, na jego wyjściu, które jest podpięte do wejścia danych karty, zastosowano prosty konwerter zbudowany z diody w kierunku zaporowym i rezystora podciągającego. Na wejściach tego układu konwertery nie są potrzebne, ponieważ odczytuje on napięcie 3 V jako stan wysoki.

## Program

Program dla mikrokontrolera został napisany w języku C z użyciem środowiska Atmel Studio. Uproszczony schemat jego działania jest przedstawiony na rys. 2. Procedury obsługi przerwań znajdują się na rys. 3, 4, 5.

**Rys. 2. Algorytm działania programu mikrokontrolera**

**Rys. 3. Algorytm obsługi przerwania Timer0 Compare**

**Rys. 4. Algorytm obsługi przerwania Pin Change**

**Rys. 5. Algorytm obsługi przerwania Timer1 Overflow**

Do obsługi systemu plików FAT na karcie SD użyto gotowej biblioteki Petit FAT FS. Jej część odpowiedzialna za komunikację przez interfejs SPI została nieznacznie zmodyfikowana, ponieważ karta jest podpięta w niestandardowy sposób (z uwagi na niewielką liczbę pinów mikrokontrolera).

W głównej pętli programu procesor odczytuje dane z karty, odczekuje pewną ilość czasu (tyle, ile trwa aktualna klatka) i wysyła dane do sterownika diod. Program zawiera dodatkowe warunki obsługujące naciśnięcia przycisku, powtarzanie animacji, detekcję błędów i wyświetlanie numerów na wyświetlaczu. Do odmierzania czasu zostało wykorzystane przerwanie Timer0 Compare, którego procedura obsługi wykonuje się dokładnie co 1 ms. Do obsługi przycisku wykorzystano przerwanie zewnętrzne Pin Change oraz przerwanie Timer1 Overflow, dzięki któremu stan przycisku jest sprawdzany pewną chwilę po przerwaniu Pin Change, co minimalizuje efekty drgań styków przycisku.

## Animacje

Animacje są zapisane na karcie w postaci binarnej. Jedna klatka jest skompresowana do 11 bajtów informacji, dzięki czemu liczba animacji, które można zmieścić na karcie jest ogromna.

Tymczasowym narzędziem do tworzenia animacji jest specjalnie przygotowany arkusz w programie Microsoft Office Excel. Dzięki niemu można wybrać, które diody powinny być zapalone w kolejnych klatkach (zrzut ekranu przedstawiającego jedną klatkę pokazano na rys. 6). Oprócz tego, dla każdej klatki można ustawić jej jasność, czas trwania i informację o ewentualnym powtórzeniu sekwencji. Ostatecznie dane z arkusza są konwertowane na postać binarną.

**Rys. 6. Fragment arkusza kalkulacyjnego do tworzenia animacji**

# Dalszy rozwój projektu

Aktualnie trwają prace nad programem, który usprawniłby proces tworzenia animacji. Aplikacja będzie napisana w języku C# z wykorzystaniem technologii WPF. Projektowanie animacji będzie bardzo proste dzięki intuicyjnemu interfejsowi użytkownika i widokowi trójwymiarowemu (zrzut ekranu pokazano na rys. 7).

Program będzie obsługiwał dowolny rozmiar wyświetlacza, a także diody RGB, dzięki czemu będzie można go stosować do tworzenia animacji również na inne kostki LED.

![](Photos/Led%20Cube%205.jpg)

![](Photos/Led%20Cube%206.jpg)

**Rys. 7. Fragment programu do tworzenia animacji**
