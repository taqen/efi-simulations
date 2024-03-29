load throughput_per_k.mat;
k1=density_th_normal_1;
k1=[k1; density_th_normal_2(density_th_normal_2(:,1)==1, :)];
k1=[k1; density_th_normal_3(density_th_normal_3(:,1)==1, :)];
k1=[k1; density_th_normal_4(density_th_normal_4(:,1)==1, :)];
k2=[];
k2=[k2; density_th_normal_2(density_th_normal_2(:,1)==2, :)];
k2=[k2; density_th_normal_3(density_th_normal_3(:,1)==2, :)];
k2=[k2; density_th_normal_4(density_th_normal_4(:,1)==2, :)];
k3=[];
k3=[k3; density_th_normal_2(density_th_normal_2(:,1)==3, :)];
k3=[k3; density_th_normal_3(density_th_normal_3(:,1)==3, :)];
k3=[k3; density_th_normal_4(density_th_normal_4(:,1)==3, :)];
k4=[];
k4=[k4; density_th_normal_2(density_th_normal_2(:,1)==4, :)];
k4=[k4; density_th_normal_3(density_th_normal_3(:,1)==4, :)];
k4=[k4; density_th_normal_4(density_th_normal_4(:,1)==4, :)];

avg_k1=[1 mean(k1(:,2))];
avg_k2=[2 mean(k2(:,2))];
avg_k3=[3 mean(k3(:,2))];
avg_k4=[4 mean(k4(:,2))];
avg_k_normal=[avg_k1;avg_k2;avg_k3;avg_k4];

k1=density_th_efi_1;
k1=[k1; density_th_efi_2(density_th_efi_2(:,1)==1, :)];
k1=[k1; density_th_efi_3(density_th_efi_3(:,1)==1, :)];
k1=[k1; density_th_efi_4(density_th_efi_4(:,1)==1, :)];
k2=[];
k2=[k2; density_th_efi_2(density_th_efi_2(:,1)==2, :)];
k2=[k2; density_th_efi_3(density_th_efi_3(:,1)==2, :)];
k2=[k2; density_th_efi_4(density_th_efi_4(:,1)==2, :)];
k3=[];
k3=[k3; density_th_efi_2(density_th_efi_2(:,1)==3, :)];
k3=[k3; density_th_efi_3(density_th_efi_3(:,1)==3, :)];
k3=[k3; density_th_efi_4(density_th_efi_4(:,1)==3, :)];
k4=[];
k4=[k4; density_th_efi_2(density_th_efi_2(:,1)==4, :)];
k4=[k4; density_th_efi_3(density_th_efi_3(:,1)==4, :)];
k4=[k4; density_th_efi_4(density_th_efi_4(:,1)==4, :)];

avg_k1=[1 mean(k1(:,2))];
avg_k2=[2 mean(k2(:,2))];
avg_k3=[3 mean(k3(:,2))];
avg_k4=[4 mean(k4(:,2))];
avg_k_efi=[avg_k1;avg_k2;avg_k3;avg_k4];

all_k=[];
all_k=[4 density_th_efi_4(:,2)'];
all_k=[all_k;3 density_th_efi_3(:,2)' 0];
all_k=[all_k;2 density_th_efi_2(:,2)' 0 0];
all_k=[all_k;1 density_th_efi_1(:,2)' 0 0 0];
all_k=sortrows(all_k,1);
f=figure;
hold on;
p1=plot(all_k(1:4,1), all_k(1:4,2)', 'DisplayName', 'K=1; EFI');
p1.Color='red';
p1.Marker='s';
p1.MarkerFaceColor='red';
p2=plot(all_k(2:4,1), all_k(2:4,3)', 'DisplayName', 'K=2; EFI');
p2.Color='green';
p2.Marker='d';
p2.MarkerFaceColor='green';
p3=plot(all_k(3:4,1), all_k(3:4,4)', 'DisplayName', 'K=3; EFI');
p3.Color='blue';
p3.Marker='p';
p3.MarkerFaceColor='blue';
p4=plot(all_k(4:4,1), all_k(4:4,5)', 'DisplayName', 'K=4; EFI');
p4.Color='black';
p4.Marker='o';
p4.MarkerFaceColor='black';

all_k=[];
all_k=[4 density_th_normal_4(:,2)'];
all_k=[all_k;3 density_th_normal_3(:,2)' 0];
all_k=[all_k;2 density_th_normal_2(:,2)' 0 0];
all_k=[all_k;1 density_th_normal_1(:,2)' 0 0 0];
all_k=sortrows(all_k,1);

p1=plot(all_k(1:4,1), all_k(1:4,2)', 'DisplayName', 'K=1; WIFI');
p1.Color='yellow';
p1.Marker='s';
p1.MarkerFaceColor='yellow';
p2=plot(all_k(2:4,1), all_k(2:4,3)', 'DisplayName', 'K=2; WIFI');
p2.Color='magenta';
p2.Marker='d';
p2.MarkerFaceColor='magenta';
p3=plot(all_k(3:4,1), all_k(3:4,4)', 'DisplayName', 'K=3; WIFI');
p3.Color='cyan';
p3.Marker='p';
p3.MarkerFaceColor='cyan';
p4=plot(all_k(4:4,1), all_k(4:4,5)', 'DisplayName', 'K=4; WIFI');
p4.Color=[0.5 0.5 0.5];
p4.Marker='o';
p4.MarkerFaceColor=[0.5 0.5 0.5];

title('Average throughput per K for each Maximum K simulation');
xticks([1 2 3 4]);
xlabel('K');
ylabel('Throughput (MB/s)');
legend show;

% f=figure;
% hold on;
% s1=scatter(density_th_efi_1(:,1),density_th_efi_1(:,2));
% s2=scatter(density_th_efi_2(:,1),density_th_efi_2(:,2));
% s3=scatter(density_th_efi_3(:,1),density_th_efi_3(:,2));
% s4=scatter(density_th_efi_4(:,1),density_th_efi_4(:,2));
% s1.Marker = '*';
% s2.Marker = '+';
% s3.Marker = 'x';
% s1.MarkerEdgeColor='red';
% s2.MarkerEdgeColor='green';
% s3.MarkerEdgeColor='blue';
% s4.MarkerEdgeColor='black';
% s1.MarkerFaceColor='red';
% s2.MarkerFaceColor='green';
% s3.MarkerFaceColor='blue';
% s4.MarkerFaceColor='black';
% 
% p1 = plot(density_th_efi_1(:,1),density_th_efi_1(:,2));
% p2 = plot(density_th_efi_2(:,1),density_th_efi_2(:,2));
% p3 = plot(density_th_efi_3(:,1),density_th_efi_3(:,2));
% p4 = plot(density_th_efi_4(:,1),density_th_efi_4(:,2));
% p1.Color='red';
% p2.Color='green';
% p3.Color='blue';
% p4.Color='black';

f = figure;
hold on;
plot(avg_k_efi(:,1),avg_k_efi(:,2), 'DisplayName' , 'EFI');
plot(avg_k_normal(:,1),avg_k_normal(:,2), 'DisplayName', 'WIFI');
title('Average throughput for all clients');
xticks([1 2 3 4]);
xlabel('K max')';
ylabel('Throughput (MB/s)');
legend show;