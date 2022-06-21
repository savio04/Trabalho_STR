arquivo = fopen('dados.txt');
v = fscanf(arquivo,'%f');

minimo_tempo_resposta = min(v);%%Tempo de resposta minimo
media_tempo_resposta = mean(v); %Tempo de resposta m dio
HWM = max(v);%High Water Mark - Tempo de resposta maximo

newData = v(:,1); % import Y dos dados dos sensores
newData(newData < 50) = 0; %conjunto com amostras com tempo de execu��o maior que o deadline
dataset_new = newData;

amostras_deadline = nnz(~dataset_new)%n�mero de amostras que cumpriram o deadline
%for i = 1 : length(dataset_new)
     %if dataset_new(i:1) == 0
         %skip = skip + 1;
     %else
         %skip = 0;
%end

%subplot(2,1,1)
bar(dataset_new)
axis([0 length(dataset_new) 0 1.1*max(dataset_new)])
xlabel('Casos de Teste com tempo de execu��o > 50 ms','fontweight','bold','fontsize',12)
ylabel('Tempo de Execu��o [ms]','fontweight','bold','fontsize',12)