%Extrai dados de arquivo
arquivo = fopen('dados.txt');
v = fscanf(arquivo,'%f');

%%Tempo de execução médio
tem_exec_ref = mean(v); %us
minimo_tempo_resposta = min(v);
figure();
%Casos de Teste
subplot(2,1,1)
plot(1:length(v),v,1:length(v),tem_exec_ref.*ones(length(v),1))
axis([0 length(v) 0 1.1*max(v)])
xlabel('Casos de Teste','fontweight','bold','fontsize',12)
ylabel('Tempo de Execução [us]','fontweight','bold','fontsize',12)

%Máximo de Blocos (BM)
x=[];
bloco = 1;
for i=0:bloco:(length(v)-bloco)
x=[x max(v(i+1:i+bloco))];
end
xmb=sort(x);

%Histograma
subplot(2,1,2)
[h,x]=hist(xmb,[min(xmb):1:max(xmb)]);
bar(x,h);
colormap lines
axis([0.8*(min(xmb)) 1.1*max(xmb) 0 1.1*max(h)])
xlabel('Tempo de Execução [us]','fontweight','bold','fontsize',12)
ylabel('Frequência','fontweight','bold','fontsize',12)