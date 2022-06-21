# Software em Tempo Real
## 📘 Controlador de um sistema de caldeira
### 🎲 Executando o Projeto

**1° passo *(no terminal)*:**
- Executar o simulador do aquecedor `Aquecedor2021.jar` com duplo clique (terá porta padrão 4545).
- Ou executar com o script `java -jar .\Aquecedor2021.jar XXXX`, onde `XXXX` é a porta em que se deseja executar.
> Obs.: Lembre-se de estar no diretório do arquivo.

**2° passo *(no terminal)*:**
- Gerar o código objeto do controlador com o script `gcc main.c sensores.c socket.c tela.c referenciaT.c referenciaH.c bufduplo.c -o index.o -lpthread`.

**3° passo *(no terminal)*:**
- Executar o código objeto do controlador com o script `sudo ./index.o localhost XXXX` oou `./index.o localhost XXXX`, onde `XXXX` é a porta em que está executando o simulador (se for a padrão é 4545).

---