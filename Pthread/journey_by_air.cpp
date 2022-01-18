#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <chrono>

using namespace std;

double arrival_rate = 15;
double total_time = 30; // to generate passenger for n seconds

bool to_console = false;

ofstream out("passenger_info.txt");

chrono::steady_clock::time_point begin_time;

struct Passenger{
    int p_id;
    bool is_vip;
    bool has_boarding_pass;
    string vip_status;
};

//globals

int vip = 3; //vip frequency 1 in vip
int lose = 3; //losing pass frequency 1 in lose

pthread_mutex_t printing;

int kiosk_num, belt_num, belt_cap, kiosk_time,sec_chk_time, board_time, vip_chnl_time;
sem_t kiosks;
vector<bool> kiosk_empty_state;
pthread_mutex_t kiosk_select;
pthread_mutex_t kiosk_deselect;

vector<sem_t> belts;
pthread_mutex_t belt_select;

pthread_mutex_t lose_pass;
pthread_mutex_t boarding;

pthread_mutex_t channel_mutex1;
pthread_mutex_t channel_mutex2;
pthread_mutex_t channel_dir;
pthread_mutex_t priority;
int l_to_r_count = 0;
int r_to_l_count = 0;

pthread_mutex_t recollect;

int passenger_count = 0;
vector<Passenger> passenger_info;

void self_check(int id)
{
    chrono::steady_clock::time_point end;

    sem_wait(&kiosks); //wait if no kiosk empty

    pthread_mutex_lock(&kiosk_select); //kiosk select atomic operation

    int i;
    for(i=1; i <= kiosk_num; i++)
    {
        if(kiosk_empty_state[i]==true) break;
    }
    kiosk_empty_state[i] = false;

    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started self check in kiosk "<<i<<" at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started self check in kiosk "<<i<<" at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);
    
    pthread_mutex_unlock(&kiosk_select);

    sleep(kiosk_time);

    pthread_mutex_lock(&kiosk_deselect); //kiosk deselect atomic operation

    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has finished self check in kiosk "<<i<<" at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has finished self check in kiosk "<<i<<" at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);

    kiosk_empty_state[i] = true;

    pthread_mutex_unlock(&kiosk_deselect);

    sem_post(&kiosks);
}

void security_check(int id)
{
    chrono::steady_clock::time_point end;

    pthread_mutex_lock(&belt_select); //belt select atomic 
    
    random_device rand_dev;
    mt19937 generator(rand_dev());
    uniform_int_distribution<int> distr(1,belt_num);

    int selected_belt = distr(generator);

    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started waiting for security check in belt "<<selected_belt<<" at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started waiting for security check in belt "<<selected_belt<<" at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);

    pthread_mutex_unlock(&belt_select);

    sem_wait(&belts[selected_belt]);

    //pthread_mutex_lock(&checking);

    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started security check in belt "<<selected_belt<<" at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started security check in belt "<<selected_belt<<" at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);
    sleep(sec_chk_time);
    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has finished security check in belt "<<selected_belt<<" at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has finished security check in belt "<<selected_belt<<" at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);

    sem_post(&belts[selected_belt]);
}

bool board(int id)
{
    chrono::steady_clock::time_point end;

    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started waiting to be boarded at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started waiting to be boarded at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);

    pthread_mutex_lock(&lose_pass);

    random_device rand_dev;
    mt19937 generator(rand_dev());
    uniform_int_distribution<int> distr(1,100);

    if(distr(generator)%lose==0)
    {
        passenger_info[id].has_boarding_pass = false;
    }
    else passenger_info[id].has_boarding_pass = true;

    pthread_mutex_unlock(&lose_pass);

    pthread_mutex_lock(&boarding);

    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started boarding the plane at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started boarding the plane at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);

    sleep(board_time);

    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(passenger_info[id].has_boarding_pass)
    {
        if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has boarded the plane at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
        out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has boarded the plane at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    }
    else 
    {
        if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" doesn't have boarding pass. Sent to vip channel at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
        out<<"Passenger "<<id<<passenger_info[id].vip_status<<" doesn't have boarding pass. Sent to vip channel at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    }
    pthread_mutex_unlock(&printing);

    pthread_mutex_unlock(&boarding);

    if(!passenger_info[id].has_boarding_pass) return false;
    return true;
}

void walk_through_vip_channel_l_to_r(int id)
{
    chrono::steady_clock::time_point end;

    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started waiting in front of vip channel to boarding gate at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started waiting in front of vip channel to boarding gate at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);

    pthread_mutex_lock(&channel_mutex1);

    l_to_r_count+=1;
    if(l_to_r_count==1)
    {
        pthread_mutex_lock(&priority);
        pthread_mutex_lock(&channel_dir);
        
    }
    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started moving on vip channel to boarding gate at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started moving on vip channel to boarding gate at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);
    
    pthread_mutex_unlock(&channel_mutex1);

    sleep(vip_chnl_time);

    pthread_mutex_lock(&channel_mutex1);

    l_to_r_count-=1;
    if(l_to_r_count==0)
    {
        pthread_mutex_unlock(&channel_dir);
        pthread_mutex_unlock(&priority);
    }
    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has got down from vip channel to boarding gate at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has got down from vip channel to boarding gate at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);
    
    pthread_mutex_unlock(&channel_mutex1);

}

void walk_through_vip_channel_r_to_l(int id)
{
    chrono::steady_clock::time_point end;

    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started waiting in front of vip channel to special kiosk at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started waiting in front of vip channel to special kiosk at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);

    pthread_mutex_lock(&priority);
    pthread_mutex_unlock(&priority);

    pthread_mutex_lock(&channel_mutex2);

    r_to_l_count+=1;
    if(r_to_l_count==1)
    {
        pthread_mutex_lock(&channel_dir);
        
    }
    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started moving on vip channel to special kiosk at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started moving on vip channel to special kiosk at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);
    
    pthread_mutex_unlock(&channel_mutex2);

    sleep(vip_chnl_time);

    pthread_mutex_lock(&channel_mutex2);

    r_to_l_count-=1;
    if(r_to_l_count==0)
    {
        pthread_mutex_unlock(&channel_dir);
    }
    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has got down from vip channel to special kiosk at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has got down from vip channel to special kiosk at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);
    
    pthread_mutex_unlock(&channel_mutex2);
    
}

void recollect_pass(int id)
{
    chrono::steady_clock::time_point end;

    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started waiting at line behind the special kiosk at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started waiting at line behind the special kiosk at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);

    pthread_mutex_lock(&recollect);

    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started waiting at the special kiosk at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has started waiting at the special kiosk at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);

    sleep(kiosk_time);

    pthread_mutex_lock(&printing);
    end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<id<<passenger_info[id].vip_status<<" has recollected boarding pass at the special kiosk at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<id<<passenger_info[id].vip_status<<" has recollected boarding pass at the special kiosk at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);

    pthread_mutex_unlock(&recollect);
}

void* init_journey(void *passenger_id)
{
    int p_id = (intptr_t)passenger_id;

    pthread_mutex_lock(&printing);
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    if(to_console) cout<<"Passenger "<<p_id<<passenger_info[p_id].vip_status<<" has arrived at the airport at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    out<<"Passenger "<<p_id<<passenger_info[p_id].vip_status<<" has arrived at the airport at time "<< chrono::duration_cast<chrono::seconds>(end-begin_time).count() <<endl;
    pthread_mutex_unlock(&printing);

    self_check(p_id);
    if(passenger_info[(p_id)].is_vip){ walk_through_vip_channel_l_to_r(p_id); }
    else security_check(p_id);
    while(!board(p_id))
    {
        walk_through_vip_channel_r_to_l(p_id);
        recollect_pass(p_id);
        walk_through_vip_channel_l_to_r(p_id);
    }
}

int main()
{
    //timer
    begin_time = chrono::steady_clock::now();
    sleep(1);

    ifstream file("in.txt");
    vector<int> values;
    string temp;

    while(getline(file, temp))
    {
        stringstream ss(temp);
        int value;
        
        while(ss>>value)
        {
            values.push_back(value);
        }
    }

    //initialize airport variables5
    kiosk_num = values[0];
    for(int i = 0; i < kiosk_num+1; i++)
    {
        kiosk_empty_state.push_back(true);
    }
    belt_num = values[1];
    belt_cap = values[2];
    kiosk_time = values[3];
    sec_chk_time = values[4];
    board_time = values[5];
    vip_chnl_time = values[6];

    //init semaphores
    pthread_mutex_init(&printing, NULL);

    sem_init(&kiosks, 0, kiosk_num);

    for(int i = 0; i <= belt_num; i++)
    {
        sem_t temp;
        belts.push_back(temp);
        sem_init(&belts[i],0,belt_cap);
    }

    pthread_mutex_init(&kiosk_select, NULL);
    pthread_mutex_init(&belt_select, NULL);
    pthread_mutex_init(&kiosk_deselect, NULL);
    pthread_mutex_init(&lose_pass, NULL);
    pthread_mutex_init(&boarding, NULL);
    pthread_mutex_init(&channel_mutex1, NULL);
    pthread_mutex_init(&channel_mutex2, NULL);
    pthread_mutex_init(&channel_dir, NULL);
    pthread_mutex_init(&priority, NULL);
    pthread_mutex_init(&recollect, NULL);

    passenger_info.push_back({NULL, NULL, NULL});

    //poisson
    random_device poisson_rd;
    mt19937 rng(poisson_rd());

    double lambda = arrival_rate/60.0;
    //cout<<lambda<<endl;
    exponential_distribution<double> pois_exp(lambda);

    int sum_time = 0;
    int new_time = 0;


    while(true)
    {
        pthread_t passenger_thread;
        passenger_count++;

        Passenger p1 = {passenger_count, false, true, ""};

        //generate vip
        random_device rand_dev;
        mt19937 generator(rand_dev());
        uniform_int_distribution<int> distr(1,100);

        if(distr(generator)%vip==0)
        {
            p1.is_vip = true;
            p1.vip_status = " (vip)";
        }

        passenger_info.push_back(p1);
        pthread_create(&passenger_thread, NULL, init_journey, (void*)p1.p_id);

        new_time = round(pois_exp.operator() (rng));
        //cout<<"new time "<<new_time<<endl;
        sum_time+=new_time;
        sleep(new_time);
        //if(passenger_count==6) break;
        if(sum_time >= total_time ) break;
    }
    pthread_exit(NULL);
    return 0;

}
