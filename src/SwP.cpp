#include<bits/stdc++.h>
#define MP make_pair
#define X first
#define Y second
using namespace std;
typedef long double ld;
typedef pair<ld, ld> pdd;

const int MAXN=100000, INF=1e9, EPS=1e-9;
// Number of job
int n;
// SwP parameters
ld lambda, mu;
// Prediction of release time and deadline of each job
ld p[MAXN], q[MAXN];
// Real release time and deadline and workload of each job
ld r[MAXN], d[MAXN], w[MAXN];
// Speed of each interval in SwP, left side and right side
ld speed_l[MAXN], speed_r[MAXN];

// Struct of job, containing job number, release time, deadline, and workload of a job
struct job {
  int index;
  ld release_time, deadline, workload;
  job(int _i, ld _r, ld _d, ld _w) {
    index = _i;
    release_time = _r;
    deadline = _d;
    workload = _w;
  }
  job(const job& J) {
    index = J.index;
    release_time = J.release_time;
    deadline = J.deadline;
    workload = J.workload;
  }
};

// Sort jobs based on their release time
bool cmp_release_time(job A, job B) {
  if(A.release_time == B.release_time)
    return A.deadline < B.deadline;
  return A.release_time < B.release_time;
}

// Step 1: Shrink predictions according to lambda
void shrink_prediction()
{
  for(int i=0;i<n;i++)
    {
      ld l = q[i] - p[i];
      p[i] = p[i] + l*lambda;
      q[i] = q[i] - l*lambda;
    }
}

// Step 2: Run YDS on shrinked predictions
vector<job> run_YDS(vector<job> input_jobs)
{
  // Speed of each job in YDS
  ld speed[MAXN];
  // Copy jobs
  vector<job> jobs;
  for(int i=0;i<n;i++)
    jobs.push_back(job(input_jobs[i]));
  
  // The main part of YDS, here we find the critical interval in each step
  // and assign speed of all jobs inside that interval
  while(jobs.size()> 0)
    {
      // Store all release times and deadlines sorted as important times
      vector<ld> times;
      for(int i=0;i<jobs.size();i++) {
	times.push_back(jobs[i].release_time);
	times.push_back(jobs[i].deadline);
      }
      sort(times.begin(), times.end());
      times.resize(unique(times.begin(), times.end()) - times.begin());
      // Now, for each interval between important times, we compute the total workload of
      // all jobs which are completely inside that interval. Then, we choose the interval
      // that has a maximum amount of total workload over its length
      ld best_interval_w = 0, best_interval_l = -1, best_interval_r = -1;
      for (int i = 0; i < times.size(); i ++)
	for (int j = i+1; j < times.size(); j ++)
	  {
	    ld interval_workload = 0;
	    // Sum workload of jobs inside interval [times[i], times[j]]
	    for (int k = 0; k < jobs.size(); k ++)
	      if (times[i] <= jobs[k].release_time && jobs[k].deadline <= times[j])
		interval_workload += jobs[k].workload;
	    // Check if interval [times[i], times[j]] is the critical interval
	    if(best_interval_w == 0 || interval_workload * (best_interval_r - best_interval_l) > (times[j] - times[i]) * best_interval_w) {
	      best_interval_w = interval_workload;
	      best_interval_l = times[i];
	      best_interval_r = times[j];
	    }
	  }
      
      // Assign speed of all jobs inside the critical interval, then shrink the critical interval
      vector<job> saved_jobs;
      for(int i=0;i<jobs.size();i++)
	{
	  // Assign speed of all jobs inside the critical interval and remove them
	  if(best_interval_l <= jobs[i].release_time && jobs[i].deadline <= best_interval_r)
	    speed[jobs[i].index] = best_interval_w / (best_interval_r - best_interval_l);
	  else {
	    // Update release time of job after shrinking the critical interval
	    if(jobs[i].release_time >= best_interval_l)
	      jobs[i].release_time = max(best_interval_l, jobs[i].release_time - (best_interval_r - best_interval_l));
	    // Update deadline of job after shrinking the critical interval
	    if(jobs[i].deadline >= best_interval_l)
	      jobs[i].deadline = max(best_interval_l, jobs[i].deadline - (best_interval_r - best_interval_l));
	    // Save this job because it was not completely inside the critical interval
	    saved_jobs.push_back(jobs[i]);
	  }
	}
      jobs = saved_jobs;
    }

  // Now for each job, we are going to determine what intervals we run this job.
  // We assign jobs in according to Earliest Deadline First
  for(int i=0;i<n;i++)
    jobs.push_back(job(input_jobs[i]));
  // Sort jobs according to their release time
  sort(jobs.begin(), jobs.end(), cmp_release_time);
  // todo has all jobs which released and has not done yet
  vector<job> todo;
  // result will contain all intervals assigned to each job
  vector<job> result;
  // Assigning jobs to different intervals in chronological order of intervals
  for(int i=0;i<n;i++) {
    // jobs[i] released
    todo.push_back(jobs[i]);
    ld next_release = (i<n-1? jobs[i+1].release_time: INF);
    if(next_release == jobs[i].release_time)
      continue;
    // Store how much time assigned after jobs[i] released 
    ld sum_runtime = 0.0;
    while(todo.size())
      {
	// Find job with earliest deadline in todo
	int first_deadline = 0;
	for(int j=1;j<todo.size();j++)
	  if(todo[j].deadline < todo[first_deadline].deadline)
	    first_deadline = j;
	// Now todo[first_deadline] is job with earliest deadline, we find how much time it needs to be done
	ld runtime = todo[first_deadline].workload / speed[todo[first_deadline].index];
	if (jobs[i].release_time + sum_runtime + runtime <= next_release) {
	  // If we have enough time to run the job with earliest deadline, we run it completely and add this interval to result
	  result.push_back(job(todo[first_deadline].index, jobs[i].release_time + sum_runtime, jobs[i].release_time + sum_runtime + runtime, todo[first_deadline].workload));
	  todo.erase(todo.begin() + first_deadline);
	  sum_runtime += runtime;
	}
	else {
	  // If we don't have enough time to run the job with earliest deadline (before releasing next job), we do this job as far as possible
	  // and keep remaining workload of this job. Then we release next job
	  ld done_workload = (next_release - (jobs[i].release_time + sum_runtime)) * speed[todo[first_deadline].index];
	  result.push_back(job(todo[first_deadline].index, jobs[i].release_time + sum_runtime, next_release, done_workload));
	  todo[first_deadline].workload -= done_workload;
	  break;
	}
      }
  }
  // This is an unnecessary part for cleaning result, here we merge consecutive intervals which assigned to same job
  vector<job> tmp;
  for(int i=0;i<result.size();i++)
    {
      if(tmp.size() == 0 || tmp.back().index != result[i].index)
	tmp.push_back(result[i]);
      else {
	tmp.back().deadline = result[i].deadline;
	tmp.back().workload += result[i].workload;
      }
    }
  result = tmp;
  
  return result;
}

// Step 3.1: Fill gaps between intervals, assign these gaps to dummy job with id=-1
vector<job> fill_gaps(vector<job> workloads, ld first_release, ld last_deadline)
{
  vector<job> result;
  // Fill empty interval from first real release time until first assigned interval
  if(first_release < workloads[0].release_time)
    result.push_back(job(-1, first_release, workloads[0].release_time, 0));
  // Fill gaps between consecutive assigned jobs
  for(int i = 0; i < workloads.size() - 1; i ++) {
    result.push_back(workloads[i]);
    // Fill empty interval from workloads[i]'s deadline until workloads[i+1]'s release time
    if(workloads[i].deadline + EPS < workloads[i+1].release_time)
      result.push_back(job(-1, workloads[i].deadline, workloads[i+1].release_time, 0));
  }
  result.push_back(workloads.back());
  // Fill empty interval from last assigned interval until last real deadline
  if(workloads.back().deadline < last_deadline)
    result.push_back(job(-1, workloads.back().deadline, last_deadline, 0));

  return result;
}

// Step 3.2: Break intervals from release times and deadlines to run SwP easily
vector<job> break_intervals(vector<job> filled_intervals, vector<ld> real_times)
{
  // We are going to break filled_intervals with real_times
  int p_real_times = 0;
  vector<job> result;
  for(int i=0;i<filled_intervals.size();)
    {
      // Skip real_times if it is before this interval 
      while(p_real_times < real_times.size() && real_times[p_real_times] <= filled_intervals[i].release_time)
	p_real_times ++;
      if(p_real_times < real_times.size() && real_times[p_real_times] < filled_intervals[i].deadline) {
	// Break this interval if real_times[p_real_times] is inside this interval and go to next real_times
	ld done_workload = filled_intervals[i].workload * (real_times[p_real_times] - filled_intervals[i].release_time) / (filled_intervals[i].deadline - filled_intervals[i].release_time);
	result.push_back(job(filled_intervals[i].index, filled_intervals[i].release_time, real_times[p_real_times], done_workload));
	filled_intervals[i].release_time = real_times[p_real_times];
	filled_intervals[i].workload -= done_workload;
	p_real_times ++;
      }
      else {
	// If there aren't any more real_times in this interval, go to next interval
	result.push_back(filled_intervals[i]);
	i++;
      }
    }
  return result;
}

// Find maximum speed of intervals which will running a special job with amount of workload equals to workload
// This function in addition of workload, get total length of left parts assigned to this job
// and pair of (current speed, length) of right parts lies inside this job's interval
// and also maximum amount of change could happed on right parts
ld find_max_speed(ld workload, ld len_l, vector<pdd> intervals, ld max_change_r)
{
  // Sort right parts
  sort(intervals.begin(), intervals.end());
  // The case which all workload will run on left parts
  if(intervals[0].X * len_l >= workload)
    return workload / len_l;
  // Run workload on left part until its speed become at least equals to right part with minimum speed
  workload -= intervals[0].X * len_l;
  // We are going to iterate over right parts and add each right part if we should run this job with speed more than pre speed of this right part
  // the p2 variable is index of first right part which should be checked if we run this job with speed more than pre speed of this right part
  // the p1 variable is index of first right part which we may exceed its maximum limit of workloads it can receive according to max_change_r
  int p1 = 0, p2 = 1;
  // First, we just have left sides and first right side
  ld sum_len = len_l + intervals[0].Y;
  while(true) {
    // If intervals[p1] has reached its limit, we return its extra workload
    if(p1 < p2 - 1 && intervals[p1].X + max_change_r < intervals[p2 - 1].X) {
      workload += (intervals[p2 - 1].X - (intervals[p1].X + max_change_r)) * intervals[p1].Y;
      sum_len -= intervals[p1].Y;
      p1++;
    }
    // Else, if our speed for this job will exceed more than pre speed of this right part, add this right part and work until pre speed of this right part
    else if(p2 < intervals.size() && (intervals[p2].X - intervals[p2 - 1].X) * sum_len <= workload) {
      workload -= (intervals[p2].X - intervals[p2 - 1].X) * sum_len;
      sum_len += intervals[p2].Y;
      p2++;
    }
    // Else, if we are not going to add intervals[p2] and doing other workload will exceed speed of intervals[p1], run job in intervals[p1] until its limit and remove it
    else if(p1 < p2 - 1 && intervals[p1].X + max_change_r < intervals[p2 - 1].X + workload / sum_len) {
      workload -= (intervals[p1].X + max_change_r - intervals[p2 - 1].X) * intervals[p1].Y;
      sum_len -= intervals[p1].Y;
      p1++;
    }
    // Else, do remaining workload in remaining righ parts
    else
      return intervals[p2-1].X + workload / sum_len;
  }
}

// Step 4: Run SwP on real jobs according to calculated scheduling for predictions
void run_SwP(vector<job> workloads, vector<job> jobs)
{
  // Release jobs in chronological order of their release time
  for(int i=0;i<jobs.size();i++)
    {
      // Store sum of all left parts which assigned to jobs[i] in YDS
      ld len_l = 0;
      // Store pair of (current speed, length) of right parts which lied inside jobs[i] interval 
      vector<pdd> rights;
      for(int j=0;j<workloads.size();j++)
	if(jobs[i].release_time <= workloads[j].release_time && workloads[j].deadline <= jobs[i].deadline) {
	  // Add left part of this interval if it is assigned to jobs[i] in YDS
	  if(jobs[i].index == workloads[j].index)
	    len_l += (1.0 - mu) * (workloads[j].deadline - workloads[j].release_time);
	  // Add right part of this interval if it lies inside jobs[i] interval
	  rights.push_back(MP(speed_r[j], mu * (workloads[j].deadline - workloads[j].release_time)));
	}
      
      // This is maximum change in speed for right parts according to AVR
      ld max_change_r = jobs[i].workload / (mu * (jobs[i].deadline - jobs[i].release_time));
      // Find maximum speed of intervals which will running jobs[i]
      ld max_speed = find_max_speed(jobs[i].workload, len_l, rights, max_change_r);

      // Now we update speed of each interval which will running jobs[i]
      for(int j=0;j<workloads.size();j++)
	if(jobs[i].release_time <= workloads[j].release_time && workloads[j].deadline <= jobs[i].deadline) {
	  // Set speed of left parts to max_speed if it is assigned to jobs[i]
	  if(jobs[i].index == workloads[j].index)
	    speed_l[j] = max_speed;
	  // If this interval lies in jobs[i] interval, we may run some workload of jobs[i] in right part of this interval
	  // The amount of workload in this part won't change speed of this part more than max_change_r and
	  // its speed won't exceed max_speed
	  speed_r[j] = max(min(max_speed, speed_r[j] + max_change_r), speed_r[j]);
	}
    }
}

int main()
{
  // Read Input
  cin>>n;
  for(int i=0;i<n;i++)
    cin>>p[i]>>q[i]>>r[i]>>d[i]>>w[i];
  cin>>lambda>>mu;
  // Step 1: shrink predictions
  shrink_prediction();
  // Step 2: Run YDS on shrinked predictions
  vector<job> shrinked_predictions;
  for(int i=0;i<n;i++)
    shrinked_predictions.push_back(job(i, p[i], q[i], w[i]));
  vector<job> after_yds = run_YDS(shrinked_predictions);
  // Step 3: Clear data before running SwP algorithm
  // Step 3.1: Fill gaps between intervals, assign these gaps to dummy job with id=-1
  vector<ld> real_times;
  for (int i = 0; i < n; i ++) {
    real_times.push_back(r[i]);
    real_times.push_back(d[i]);
  }
  sort(real_times.begin(), real_times.end());
  real_times.resize(unique(real_times.begin(), real_times.end()) - real_times.begin());
  // We pass first release time and last deadline and all assigned intervals to this function
  // it will assign all empty intervals from first release time to last deadline to a dummy job
  vector<job> filled_intervals = fill_gaps(after_yds, real_times[0], real_times.back());
  // Step 3.2: Break intervals from release times and deadlines to run SwP easily
  vector<job> cleaned_intervals = break_intervals(filled_intervals, real_times);
  // Step 4: Run SwP on real jobs according to calculated scheduling for predictions
  vector <job> real_jobs;
  for (int i = 0; i < n; i ++)
    real_jobs.push_back(job(i, r[i], d[i], w[i]));
  sort(real_jobs.begin(), real_jobs.end(), cmp_release_time);
  run_SwP(cleaned_intervals, real_jobs);
  // Calculate consumed energy
  ld ans = 0;
  for(int i=0;i<cleaned_intervals.size();i++)
    {
      ld ll = cleaned_intervals[i].deadline - cleaned_intervals[i].release_time;
      // Energy consumed in cleaned_intervals[i]
      ans += speed_l[i] * speed_l[i] * speed_l[i] * ll * (1-mu) + speed_r[i] * speed_r[i] * speed_r[i] * ll * mu;
    }
  cout<<fixed<<ans<<endl;
}

