# Progetto di Sistemi Operativi

**Anno Accademico 2022-2023**  
**Bacci Manuel** - 0001040090 - [manuel.bacci@studio.unibo.it](mailto:manuel.bacci@studio.unibo.it)  
**Shanshan Feng** - 0000890555

---

## PandOS Livello 2

Il progetto consiste nello sviluppo del secondo livello del sistema operativo didattico **Panda+**, progettato per l’architettura **uMPS3**, che si articola su sei livelli di astrazione. Questo livello, denominato livello delle code, si occupa dell’implementazione delle principali strutture dati e delle funzionalità associate ai **Process Control Block (PCB)**. Tra le operazioni gestite vi sono l’allocazione e la deallocazione dei PCB, la gestione delle code e dell’albero dei processi, il controllo delle strutture legate ai semafori (Active Semaphore Hash) e la gestione dei namespace dei processi.

---

## Requisiti

Il progetto è compatibile esclusivamente con sistemi operativi **Linux**. È inoltre richiesto l’emulatore **uMPS3**, il quale può essere installato attraverso il seguente comando da terminale:

```bash
sudo apt install umps3
```

## Compilazione

Dopo aver scaricato l’archivio **phase1.tar.gz**, è necessario accedere alla directory contenente il file, estrarne il contenuto ed eseguire la compilazione del progetto. È possibile utilizzare due modalità: tramite Makefile oppure attraverso CMake.

Per iniziare:

```shell
tar -xvzf phase1.tar.gz
cd phase1
```

### Makefile

Per compilare tramite Makefile, è sufficiente eseguire il comando:

```shell
make
```

### CMake

In alternativa, è possibile configurare l’ambiente di build con CMake utilizzando:

```shell
mkdir build
cd build
cmake -D CMAKE_TOOLCHAIN_FILE=../umps.cmake ..
make
```

## Esecuzione

Una volta compilato il progetto, è possibile avviarlo tramite l’interfaccia grafica dell’emulatore uMPS3, disponibile dal menu delle applicazioni o eseguibile direttamente da terminale con il comando:

```shell
umps3
```

All’avvio, si dovrà creare una nuova configurazione del simulatore selezionando **Simulator > New Configuration**, assegnando un nome identificativo e specificando la directory in cui è presente il file eseguibile del progetto (**phase1_files** o equivalente). Completata la configurazione, sarà possibile accendere la macchina virtuale con **Power On** ed eseguirne l’avvio con **Continue**. L’output verrà mostrato all’interno della console, accessibile dal menu **Windows > Terminal 0**. In sintesi:

```shell
umps3
Simulator > New Configuration
Create in: ***/phase1_files
Power On
Continue
Windows > Terminal 0
```
