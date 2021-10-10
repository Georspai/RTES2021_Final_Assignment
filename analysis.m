%Spaias Georgios
%AEM: 8910
%RTES Final Assignment

fileID=fopen('BTnearMe.bin','r');
call_t=fread(fileID, 'double');
fclose(fileID);

delay=zeros(size(call_t));
iteration_t=zeros(size(call_t));
iteration_delay=zeros(size(call_t));

%% Total delay of execution to theoretical call times
for i=1:length(call_t)
    delay(i)=call_t(i)-0.1*(i-1)-call_t(1);  
end

%% Execution time and delay of each iteration 
iteration_t(1)=call_t(1);
for i=2:length(call_t)
    iteration_t(i)= call_t(i)-call_t(i-1);
    iteration_delay(i)=iteration_t(i)-0.1;
end
y=0:(size(call_t)-1);
time=(0:(size(call_t)-1))*0.1;
avg_t=mean(iteration_t);
max_t=max(iteration_t);

avg_delay=mean(iteration_delay);
max_delay=max(iteration_delay);


%% Plots
figure;
plot(y,call_t*1000,'c',y,time*1000,'r');
title('Real vs Theoretical Operation time');
xlabel('Iterations');
ylabel('Operating time(msec)');

figure;
plot(time,delay);
title('Time Delay');
xlabel('Operating time(sec)');
ylabel('Total delay (sec)');

figure;
plot(iteration_t,'.m');
title('Time for each Iteration (sec)');
xlabel('Iterations');
ylabel('Call time (secs)');

figure;
plot(iteration_delay,'.b');
title('Delay of each Iteration');
xlabel('Iterations');
ylabel('Call time (sec)');

