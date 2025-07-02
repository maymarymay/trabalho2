O objetivo do trabalho é fazer testes comparativos hash duplo vs hash simples em uma base de dados de CEPs

Será adaptado: um código para que ele suporte hash simples e hash duplo e a estrutura da tabela hash para que armazene a taxa de ocupação da tabela. 

Será alterada: a inserção para que ao atingir esta taxa de ocupação, o tamanho da tabela seja duplicada para suportar mais inserções e a estrutura para que dado os primeiros 5 digitos do CEP ele retorne o nome da cidade e o estado.

Comparativos:
- Calcular o tempo de busca na Hash com taxa de ocupação de 10, 20, 30, 40, 50, 60, 70, 80, 90 e 99 % de ocupação.  Para este experimento considere uma tabela com 6100 buckets, faça funções específicas para cada taxa de ocupação (ex, busca10, busca20), utilizando gprof para medir o tempo de execução de cada função. (planilhar esses resultados e mostrar um gráfico de taxa de ocupação (eixo x) versus tempo de execução (eixo y) e fazer o comparativo de hash simples e dupla.)

- Comparar o de tempo de inserção de uma tabela com 6100 buckets iniciais  e com 1000 buckets iniciais e insira todas as cidades da base. Criar funções específicas para essa comparação (insere6100 e insere1000) e  com gprof apresente o tempo de execução e calcule o overhead de utilizar uma estrutura dinâmica de inserção. O objetivo aqui é verificar o tempo tomado para crescer a estrutura. 
