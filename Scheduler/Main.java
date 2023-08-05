import java.io.*;
import java.util.*;

public class Main {
    // ArrayList of Processes that needs to be scheduled at CPU
    // The array is used to store the list of processes from an input file
    public static ArrayList<Proc> procs = new ArrayList<>();

    // The Main class implements the following schedulers
    // 1. round robin
    // 2. SRT
    // 3. Feedback
    // 4. All (runs all back tp back)
    // INPUT:
    // 1. File Name
    // 2. Scheduler to simulate
    // OUTPUT:
    // 1. Prints Scheduler results

    public enum feedback_sch_queues {
        LEVEL1,
        Level2,
        Level3;
    }

    public static void main(String[] args) throws FileNotFoundException {

        // String representation of the allowed user input
        String round_robin = "RR";
        String srt = "SRT";
        String feedback = "FB";
        String all_algo = "ALL";

        // The result computed by the Algorithm are kep in the result array
        // Process that runs at a given time epoc is added to the Array list
        ArrayList<boolean[]> result = new ArrayList<>();
        ArrayList<boolean[]> result_srt = new ArrayList<>();
        ArrayList<boolean[]> result_feedback = new ArrayList<>();

        // User Input
        System.out.println("Please enter the input file name");
        // Setup scanner for user input
        Scanner consoleIo = new Scanner(System.in);

        // The program reads in a list of processes from a tab-delimited text file.
        // Get file name that contains information about each process
        String fileTitle = consoleIo.nextLine();

        // The program takes command-line parameter that is one of the following:
        // * RR,
        // * SRT,
        // * FB,
        // * ALL
        //   If ALL is input, the program should produce output for all three
        //   (RR, SRT and FB) scheduling algorithms.
        System.out.println("Please enter the algorithm to simulate");

        // Read algorithm from user
        String algo = consoleIo.next().toString();

        // Open the input file
        Scanner input = new Scanner(new File(fileTitle));

        // Print user input parameter
        //System.out.println("User input (filename: " + fileTitle + " Algorithm to simulate: " + algo + ")");

        // Read in the input file and add the processes to the procs ArrayList
        // The format of the text file should have one line for each proc,
        // where each line has a proc name, an arrival time and a duration.
        while (input.hasNext()) {
            String next_line;
            // Read the next line
            next_line = input.nextLine();
            if (!next_line.equals("")) {
                // If there are process info on the next line
                String[] line = next_line.split("\t");
                Proc curr_proc = new Proc(line[0], Integer.parseInt(line[1]), Integer.parseInt(line[2]));
                procs.add(curr_proc);
            }
        }

        // Print all processes read
        for (int i = 0; i < procs.size(); i++) {
            //procs.get(i).print();
        }

        // Call the right algorithm handler function
        // based on user input
        if (round_robin.compareTo(algo) == 0) {
            // Call round robin scheduler
            os_sch_run_round_robin_algo(procs, result);
            
            // Display results of the round robin scheduler
            System.out.println("Displaying Results for Round Robin Algorithm:");
            os_sch_print_sch_result(procs, result);
        } else if (srt.compareTo(algo) == 0) {
            // Call SRT scheduler
            os_sch_run_srt_algo(procs, result);

            // Display results of the SRT scheduler
            System.out.println("Displaying Results for SRT Algorithm:");
            os_sch_print_sch_result(procs, result);
        } else if (feedback.compareTo(algo) == 0) {
            // Call feedback scheduler
            os_sch_run_feedback_algo(procs, result);

            // Display results of the Feedback scheduler
            System.out.println("Displaying Results for Feedback Algorithm:");
            os_sch_print_sch_result(procs, result);
        } else if (all_algo.compareTo(algo) == 0) {
            // Call all scheduler and display results back to back
            System.out.println("Displaying Results for all Algorithms:");

            // Call Round Robin scheduler and display results
            os_sch_run_round_robin_algo(procs, result);
            System.out.println("Displaying Results for Round Robin Algorithm:");
            os_sch_print_sch_result(procs, result);

            // reset the process data to the state read from the file
            // This is to enable successful run of the next scheduler
            os_sch_reset_proc_data(procs);

            // Call SRT scheduler and display results
            os_sch_run_srt_algo(procs, result_srt);
            System.out.println("Displaying Results for SRT Algorithm:");
            os_sch_print_sch_result(procs, result_srt);

            // reset the process data to the state read from the file
            // This is to enable successful run of the next scheduler
            os_sch_reset_proc_data(procs);

            // Call feedback scheduler and display results
            os_sch_run_feedback_algo(procs, result_feedback);
            System.out.println("Displaying Results for Feedback Algorithm:");
            os_sch_print_sch_result(procs, result_feedback);
        } else {
            System.out.println("Entered Algorithm is not supported!");
        }
        // We are all done!
    }

    public static void os_sch_print_sch_result(ArrayList<Proc> procs,
                                               ArrayList<boolean[]> result) {

        // Print the Process Header
        // The data is printed as follows
        // <tab> A B C D E F G ...
        System.out.print("\t");
        for (Proc p : procs) {
            // Print process char name
            System.out.print(p.get_process_name() + " ");
        }

        // Next line for start printing the process data
        System.out.println();

        int time_slot = 1;
        // for each time slot recorded in the ArrayList result
        for (boolean[] time : result) {
            // The data is printed as follows
            // <time slot <tab>> X/space for each process ...
            System.out.print(time_slot + "\t");
            // for each time slot
            for (boolean proc : time) {
                // If the process is added to the result Array, print X
                // Otherwise print <space>
                System.out.print(proc ? "X " : "  ");
            }
            time_slot++;
            System.out.println();
        }
    }

    private static void os_sch_reset_proc_data(ArrayList<Proc> procs) {
        // 
        for (Proc proc : procs) {
            proc.recover_process_original_duration();
        }
    }
    public static String os_sch_algo_to_string(String algo) {

        String round_robin = "RR";
        String srt = "SRT";
        String feedback = "FB";
        String all_algo = "ALL";

        if (round_robin.compareTo(algo) == 0) {
            return "Round Robin";
        } else if (srt.compareTo(algo) == 0) {
            return "SRT";
        } else if (feedback.compareTo(algo) == 0) {
            return "Feedback";
        } else if (all_algo.compareTo(algo) == 0) {
            return"all";
        } else {
            return "Unsupported Algorithm!";
        }
    }
    // runs the RR algorithm and displays the results
    private static void os_sch_run_round_robin_algo(ArrayList<Proc> procs,
                                       ArrayList<boolean[]> result) {

        // The project assumes a time quantum of one CPU cycle.
        // The interrupt is generated each CPU cycle.
        // The CPU cycle is simulated using a local variable (time_slot).
        // The ready job executes each quantum for duration = 1.
        int time_slot = 0;

        // When an interrupt happens:
        //•	The process running at the moment is placed at the back of the ready queue.
        //•	The process at the head of the ready queue is picked up for execution.

        // Instantiate the ready_queue.
        Queue<Proc> ready_queue = new LinkedList<>();

        // The process running at the moment is placed at the back of the ready queue.
        // Need a local variable to track
        Proc proc_currently_running = null;

        // Create a list of processes that requires execution
        // Recall: The list is read from the input file
        ArrayList<Proc> remaining_processes = new ArrayList<>(procs);

        // Schedule all processses read from the input file
        while (!remaining_processes.isEmpty()) {

            // Check all processes that are ready to be executed
            for (Proc proc : procs) {
                if (proc.get_process_arrival_time() == time_slot) {
                    // If a process is ready for execution, it joins the ready queue
                    ready_queue.add(proc);
                }
            }

            // Instantiate the list to keep track of the remaining processes
            // This list is formatted as the result array to track
            // which process is currently running
            boolean[] proc_currently_runnings = new boolean[procs.size()];

            // Check if ready queue has any process to schedule
            if (!ready_queue.isEmpty()) {
                // Remove the process from the ready queue (it needs to wait for its turn)
                proc_currently_running = ready_queue.remove();
                // Record which process was running during this time slot
                for (int i = 0; i < procs.size(); i++) {
                    // Record at the index assigned to the process in result reporting
                    proc_currently_runnings[i] =
                            procs.get(i).get_process_name().equals(proc_currently_running.get_process_name());
                }
            }

            // The process currently running is now reduced duration
            proc_currently_running.setDuration(proc_currently_running.get_process_duration() - 1);

            // The process running at the moment is placed at the back of the ready queue.
            if (remaining_processes.contains(proc_currently_running)) {
                ready_queue.add(proc_currently_running);
            }

            // Record the array of which process was running at the last time slice
            result.add(proc_currently_runnings);

            // check if the currently running proc is done running
            if (proc_currently_running.get_process_duration() == 0) {
                remaining_processes.remove(proc_currently_running);
            }

            // increment the current time step
            time_slot++;
        }

    }

    // Implements the SRT Algo
    public static void os_sch_run_srt_algo(ArrayList<Proc> procs,
                                           ArrayList<boolean[]> result) {
        // The project assumes a time quantum of one CPU cycle.
        // The interrupt is generated each CPU cycle.
        // The CPU cycle is simulated using a local variable (time_slot).
        // The ready job executes each quantum for duration = 1.
        int time_slot = 0;

        // The process that is running during this quantum
        Proc proc_currently_running = null;

        // Create a list of processes that requires execution
        // Recall: The list is read from the input file
        ArrayList<Proc> remaining_processes = new ArrayList<>(procs);

        // The SRT scheduler always chooses the process that has the shortest
        // expected remaining processing time.
        // For that reason we need a list of active processes
        // The list of active processes is formatted same way as the result list
        boolean[] active_processes = new boolean[procs.size()];
        // index of the proc w/ the shortest remaining time
        int srt_proc_index = 0xFFFF;

        // Check if the process list has any process to schedule
        while (!remaining_processes.isEmpty()) {
            // From the list of processes in the remaning proc list,
            // From the process with the shortest remaning time to execute

            // Instantiate the list to keep track of the remaining processes
            // This list is formatted as the result array to track which
            // process is currently running
            boolean[] proc_currently_runnings = new boolean[procs.size()];

            // Now fin the list of the processes that are currently active
            int srt_proc_duration = 0xFFFF;
            // Walk over the list of the active processes
            // Record index of the process with SRT
            for (int i = 0; i < active_processes.length; i++) {
                // Examine each active process to find process that is active
                Proc p = procs.get(i);
                if (p.get_process_arrival_time() == time_slot) {
                    // Process p is currently active
                    active_processes[i] = true;
                }
            }

            // From the list of active proccesses,
            // find the process w/ SRT
            for (int i = 0; i < active_processes.length; i++) {
                // Examine each active process to find process w/ SRT
                Proc p = procs.get(i);
                // If p has the shortest remaining time across all active procs
                if (active_processes[i] && p.get_process_duration() < srt_proc_duration) {
                    // and record the index of the proc p w/ SRT for far
                    srt_proc_index = i;
                    // The current duration of the shortest proccess with SRT
                    srt_proc_duration = p.get_process_duration();
                }
            }

            // run the process w/ the shortest remaining time
            if (active_processes[srt_proc_index]) {
                // Get the process using the index
                // Run the process and mark it as the current!
                proc_currently_running = procs.get(srt_proc_index);

                // reduce the duration of the currently running proc by 1
                proc_currently_running.setDuration(
                        proc_currently_running.get_process_duration() - 1);

                // Record at the index assigned to the process in result reporting
                for (int i = 0; i < procs.size(); i++) {
                    proc_currently_runnings[i] = procs.get(i).get_process_name().equals(proc_currently_running.get_process_name());
                }
            }

            // check if the currently running proc is done running
            if (proc_currently_running.get_process_duration() == 0) {
                // Remove from list of remaining processes
                remaining_processes.remove(proc_currently_running);
                // Remove from list of active processes
                active_processes[srt_proc_index] = false;
            }

            // Record the array of which process was running at the last time slice
            result.add(proc_currently_runnings);

            // increment the current time step
            time_slot++;
        }

    }

    /* Implements the Feedback Algo
    The implementation of the feedback scheduler uses a series of
    queues of decreasing priority.
        * The implementation implements the 3 level of queues

    The implementation picks up the process from the high-priority queue
    (unless it is empty)
    The implementation moves to second-priority queues (unless it is empty)
    The implementation finally moves to third-priority queues
    */
    public static void os_sch_run_feedback_algo(ArrayList<Proc> procs,
                                    ArrayList<boolean[]> result) {
        // The project assumes a time quantum of one CPU cycle.
        // The interrupt is generated each CPU cycle.
        // The CPU cycle is simulated using a local variable (time_slot).
        // The ready job executes each quantum for duration = 1.
        int time_slot = 0;

        // The implementation implements the 3 level of queues
        // Instantiate the 3 level of queues
        Queue<Integer> feedback_queue_level_1 = new LinkedList<>();
        Queue<Integer> feedback_queue_level_2 = new LinkedList<>();
        Queue<Integer> feedback_queue_level_3 = new LinkedList<>();

        // Create a list of processes that requires execution
        // Recall: The list is read from the input file
        ArrayList<Proc> remaining_processes = new ArrayList<>(procs);

        // The process that is running during this quantum
        Proc proc_currently_running = null;

        // The implementation picks up the process from the high-priority queue
        //    (unless it is empty)
        //    The implementation moves to second-priority queues (unless it is empty)
        //    The implementation finally moves to third-priority queues
        // To track this, we need a list of active processes
        // The list of active processes is formatted same way as the result list
        boolean[] active_processes = new boolean[procs.size()];
        int fb_current_proc_index = 0xFFFF;
        int fb_prev_proc_index = 0xFFFF;

        // the current queue
        int fb_current_srv_queue = -1;
        // boolean indicating if a proc arrived
        boolean a_proc_is_ready = false;

        // Check if the process list has any process to schedule
        while (!remaining_processes.isEmpty()) {
            // The implementation picks up the process from the high-priority queue
            // (unless it is empty);
            // then level 2 queue (unless it is empty)
            // and then level 3 queue

            // Instantiate the list to keep track of the remaining processes
            // This list is formatted as the result array to track which
            // process is currently running
            boolean[] proc_currently_runnings = new boolean[procs.size()];

            // First find the list of the processes that are currently active
            // Walk over the list of the active processes
            // Record index of the process
            for (int i = 0; i < active_processes.length; i++) {
                // Examine each active process to find process that is active
                Proc p = procs.get(i);
                if (p.get_process_arrival_time() == time_slot) {
                    // Process p is currently active
                    active_processes[i] = true;
                }
            }

            // From the list of active proccesses,
            // schedule them to the level 1 queue
            for (int i = 0; i < active_processes.length; i++) {
                Proc currproc = procs.get(i);
                if (active_processes[i] == true) {
                    feedback_queue_level_1.add(i);
                    a_proc_is_ready = true;
                }
            }

            // The processes in the input file are executed as follows:
            //•	Implementation always adds process to the top queue first.
            //•	The process runs once for the quantum of duration = 1.
            //•	The implementation then moves the process to the second-priority lower queue.
            //•	The process runs once for the quantum of duration = 1.
            //•	The implementation then moves the process to the third-priority queue
            //•	The process runs there until finished

            // The following code setup up the scheduler based on ran during last quantum

            if (!a_proc_is_ready) {
                // No new process is ready
                // Check current queue status of existing processes
                if (feedback_queue_level_1.isEmpty() &&
                        // level 1 q is empty
                        feedback_queue_level_2.isEmpty() &&
                        // level 2 q is empty
                        feedback_queue_level_3.isEmpty()) {
                    // level 3 q is empty
                    // If all queues are empty, the current process can run in this qamtum
                    // (if remaining time left)
                    fb_current_proc_index = fb_prev_proc_index;
                }
            }

            // This is demotion code of feedback algo
            // If a process is ready it gets into L1, L2 or L3 queue based on
            // the queue it ran during last qanutum
            if (a_proc_is_ready) {
                // A process is ready, find the queue it must run

                // Implementation always adds process to the top queue first.
                // Find which queue the process run during the last quantum
                if (fb_current_srv_queue == 1 /* LEVEL 1*/) {
                    // The process ran at level 1 queue once for the quantum of duration = 1
                    // Now add it to the level 2 queue.
                    feedback_queue_level_2.add(fb_prev_proc_index);
                } else if (fb_current_srv_queue == 2 /* LEVEL 1*/) {
                    // The process ran at level 2 queue once for the quantum of duration = 1
                    // Now add it to the level 3 queue.
                    feedback_queue_level_3.add(fb_prev_proc_index);
                } else if (fb_current_srv_queue == 3 /* LEVEL 3 */) {
                    // The process ran at level 3 queue during the last quantum
                    // It remains at lebel 3 queue
                    feedback_queue_level_3.add(fb_prev_proc_index);
                }
            }

            // The following code implements the scheduler
            // The scheduling for the Feedback scheduler implementation works as follows:
            //•	The implementation always schedules the highest queue until empty.
            //•	The implementation then processes the next highest queue until empty.
            //•	If a process arrives, it goes to the highest queue and is scheduled next.
            // get the proc to run
            if (!feedback_queue_level_1.isEmpty()) {
                // During the current quantum, the process runs at L1 queue
                // Remove it for the next run
                // (it cannot run at L1 during the next quantum)
                fb_current_proc_index = feedback_queue_level_1.remove();
                // Remamber that the process has ran at the L1 queue onces
                fb_current_srv_queue = 1 /* Level 1 */;
            } else if (!feedback_queue_level_2.isEmpty()) {
                // During the current quantum, the process runs at L2 queue
                // Remove it for the next run
                // (it cannot run at L2 during the next quantum)
                fb_current_proc_index = feedback_queue_level_2.remove();
                // Remamber that the process has ran at the L2 queue onces
                fb_current_srv_queue = 2 /* Level 1 */;
            } else if (!feedback_queue_level_3.isEmpty()) {
                // During the current quantum, the process runs at L2 queue
                // Remove it for the next run
                // It will be added back to L3 queue afterwords
                fb_current_proc_index = feedback_queue_level_3.remove();
                fb_current_srv_queue = 3 /* Level 3 */;
            }

            // Find the process that has been selected to run during this quanatum
            proc_currently_running = procs.get(fb_current_proc_index);

            // reduce the duration of the currently running proc by 1
            proc_currently_running.setDuration(
                    proc_currently_running.get_process_duration() - 1);

            // Remove the process from the queue, if it is done
            if (proc_currently_running.get_process_duration() == 0) {
                // Remove from list of remaining processes
                remaining_processes.remove(proc_currently_running);
                // Remove from list of active processes
                active_processes[fb_current_proc_index] = false;
            }

            // Record at the index assigned to the process in result reporting
            for (int i = 0; i < procs.size(); i++) {
                // Record at the index assigned to the process in result reporting
                proc_currently_runnings[i] =
                        procs.get(i).get_process_name().
                                equals(proc_currently_running.get_process_name());
            }
            // Record the array of which process was running at the last time slice
            result.add(proc_currently_runnings);

            // increment the current time step and reset a_proc_is_ready
            time_slot++;
            a_proc_is_ready = false;
        }
    }
}
