# Como reproduzir os resultados apresentados no trabalho

Há dois principais métodos de estimar as eficiências. O primeiro que vou instruir é o método de Subtração de Bandas Laterais (sideband subtraction)

## Necessário

- ROOT (versão de aplicação v6.28.04)

## Subtração de Bandas Laterais

### Ideias gerais

Os arquivos referentes a este método estão na pasta `sideband_subtraction/` enquanto arquivo a ser rodado deve ser o `macro.cpp`.

O arquivo em questão executará todo o código e carregar as configurações em `settings.cpp`. Os cortes estão descritos em `cuts.h`. Os histogramas gerados têm forma descrita em `createHistogram.h`. Os bins também são designados nele.

Para executar a primeira parte é necessário alterar poucas das configuração padrão.

### Preparação 

Crie uma pasta `data/` dentro da pasta `sideband_subtraction/`. Nela coloque os arquivos de dados: `JPsiToMuMu_mergeMCNtuple.root` e `Run2011AMuOnia_mergeNtuple.root`.

### Execução

#### Dados reais do Jpsi

Abra o arquivo `settings.cpp` e troque a variável `useFile` da linha 22 para:

```cpp
int useFile = 1;
```

Esta linha indica que o código será executado para os dados reais do Jpsi. Depois, abra o terminal na pasta `sideband_subtraction/main/` e digite:

```sh
root -l -b -q macro.cpp
```

#### Dados simulados do Jpsi

Execute para os dados simulados do Jpsi alterando a variável supracita em `settings.cpp` para:

```cpp
int useFile = 2;
```

Por fim, em `sideband_subtraction/main/` e digite:

```sh
root -l -b -q macro.cpp
```

### Resultados

Os resultados de ambos os dados podem ser visualizados em `sideband_subtraction/results/` em suas respectivas pastas.

## Ajuste de função

### Ideias gerais

Os dados de input neste código tem uma estrutura ligeiramente diferente da estrutura de dados utilizadas no `sideband_subtraction/`. Isso se deve a facilidade e eficiência de filtragem de dados utilizando o Roofit com a nova estrutura de dados. Então, os dados baixados devem ser convertidos com uma macro presente nos arquivos fornecidos. 

### Preparação

Com os arquivos baixados em `sideband_subtraction/data/`, precisamos convertê-los para arquivos que o método fitting possa ler. Para isso, entre na pasta `fitting/`e abra o arquivo `simplify_data.cpp` e garanta que entre as linhas entre 5 e 9 esteja escrito assim:

```cpp
	TFile *file0  = TFile::Open("../sideband_subtraction/data/Run2011AMuOnia_mergeNtuple.root");
	TTree *TreeAT = (TTree*)file0->Get(("tagandprobe/AnalysisTree"));
	TTree *TreePC = (TTree*)file0->Get(("tagandprobe/PlotControl"));

	TFile *fileIO = TFile::Open("DATA/TagAndProbe_Jpsi_Run2011.root","RECREATE");
```

Após, abra o terminal na pasta e nele digite:

```sh
root -l -b -q simplify_data.cpp
```

Façamos o mesmo para os dados de Monte Carlo. No arquivo `simplify_data.cpp`, substitua as linhas anteriores por:

```cpp
	TFile *file0  = TFile::Open("../sideband_subtraction/data/JPsiToMuMu_mergeMCNtuple.root");
	TTree *TreeAT = (TTree*)file0->Get(("tagandprobe/AnalysisTree"));
	TTree *TreePC = (TTree*)file0->Get(("tagandprobe/PlotControl"));

	TFile *fileIO = TFile::Open("DATA/TagAndProbe_Jpsi_MC.root","RECREATE");
```

Depois, reexecute o código no terminal:

```sh
root -l -b -q simplify_data.cpp
```

### Execução

O código principal será executado pelo arquivo `fitting/efficiency.cpp`. As primeiras linhas estão descritas abaixo:

```cpp
//Change if you need
#include "src/dofits/DoFit_Jpsi_Run.h"
//#include "src/dofits/DoFit_Jpsi_MC.h"

#include "src/create_folder.h"
#include "src/get_efficiency.h"
#include "src/make_TH1D.h"

//Which Muon Id do you want to study?
string MuonId   = "trackerMuon";
//string MuonId   = "standaloneMuon";
//string MuonId   = "globalMuon";

//Which quantity do you want to use?
string quantity = "Pt";     double bins[] = {0., 2.0, 3.4, 4.0, 4.4, 4.7, 5.0, 5.6, 5.8, 6.0, 6.2, 6.4, 6.6, 6.8, 7.3, 9.5, 13.0, 17.0, 40.};
//string quantity = "Eta";    double bins[] = {-2.4, -1.8, -1.4, -1.2, -1.0, -0.8, -0.5, -0.2, 0, 0.2, 0.5, 0.8, 1.0, 1.2, 1.4, 1.8, 2.4};
//string quantity = "Phi";    double bins[] = {-3.0, -1.8, -1.6, -1.2, -1.0, -0.7, -0.4, -0.2, 0, 0.2, 0.4, 0.7, 1.0, 1.2, 1.6, 1.8, 3.0};
```

Para obter todos os resultados que queremos, precisaremos permutar pelas 9 possibilidades de combinações entre `MuonId` e `quantity`. Em outras palavras, precisamos do Pt, Eta e Phi do trackerMuon, do standaloneMuon e do globalMuon. Todos os parâmetros já estão pré-definidos, bastando apenas comentar quando em desuso e descomentar quando em uso.

Ao fazer as modificações, estando com o terminal na pasta `fitting/`, basta executar o código com:

```sh
root -l -b -q efficiency.cpp
```

#### Dados reais do Jpsi

Para rodar os dados reais do Jpsi, as duas primeiras linhas do arquivo `fitting/efficiency.cpp` devem ser substituidas por:

```cpp
//Change if you need
#include "src/dofits/DoFit_Jpsi_Run.h"
//#include "src/dofits/DoFit_Jpsi_MC.h"
```

Lembre-se de rodar o código para os dados reais como descrito na seção superior.

#### Dados simulados do Jpsi

Para rodar os dados simulados do Jpsi, as duas primeiras linhas do arquivo `fitting/efficiency.cpp` devem ser substituidas por:

```cpp
//Change if you need
//#include "src/dofits/DoFit_Jpsi_Run.h"
#include "src/dofits/DoFit_Jpsi_MC.h"
```

Lembre-se de rodar o código para os dados simulados como descrito na seção superior.

### Resultados

OS resultados podem ser visualizados na pasta `fitting/efficiencies/efficiency/` onde os gráficos gerados estão em suas respectivas pastas e seus respectivos `.root` correspondentes.

## Pile up the efficiency graphs


## Eficiência 2D

As eficiências 2D apresentadas no trabalho são estimadas através do do método de fitting e aplicando o conceito de incertezas sistemáticas. Esta consiste em fazer pequenas variações nos gráficos de eficiência e combiná-las considerar a incerteza sistemáticas na medida.

### Dados reais do Jpsi

O arquivo utilizado para o cálculo dessas eficiências está em `fitting/plot_sys_efficiency_2d.cpp`. Algumas das primeiras linhas deste arquivo, seguem:

```cpp
// Change if you need
#include "src/dofits/DoFit_Jpsi_Run.h"
#include "src/dofits/DoFit_Jpsi_Run_2xGaus.h"
//#include "src/dofits/DoFit_Jpsi_MC.h"
//#include "src/dofits/DoFit_Jpsi_MC_2xGaus.h"

// Which Muon Id do you want to study?
string MuonId   = "trackerMuon";
//string MuonId   = "standaloneMuon";
//string MuonId   = "globalMuon";
```

Deixando as primeiras linhas como acima, basta executá-lo no terminal:

```sh
root -l -b -q plot_sys_efficiency_2d.cpp
```

Para executar sobre todos as medidas estimadas, basta permutar os MuonId comentados. Fazendo para o trackerMuon, standaloneMuon e globalMuon e sempre executando o código com o comando acima.

### Dados simulados do Jpsi

Para executar o mesmo código para os dados simulados, basta alterar as linhas correspondentes do `fitting/plot_sys_efficiency_2d.cpp` para:

```cpp
//Change if you need
//#include "src/dofits/DoFit_Jpsi_Run.h"
//#include "src/dofits/DoFit_Jpsi_Run_2xGaus.h"
#include "src/dofits/DoFit_Jpsi_MC.h"
#include "src/dofits/DoFit_Jpsi_MC_2xGaus.h"
```

e repetir o comando abaixo para o trackerMuon, standaloneMuon e globalMuon:

```sh
root -l -b -q plot_sys_efficiency_2d.cpp
```

## Resultados

Os resultados desta eficiência são guardados em `fitting/efficiencies/systematic_2D/`