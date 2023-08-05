
public class Proc {
    // The name of the process
    private String process_name;
    // The arrival time of the process
    private int process_arrival_time;
    // The original duration of the process
    private int process_original_duration;
    // The remaining duration of the process
    private int process_remaining_duration;

    public Proc (String process_name, int process_arrival_time, int duration) {
        this.process_name = process_name;
        this.process_arrival_time = process_arrival_time;
        this.process_original_duration = duration;
        this.process_remaining_duration = duration;
    }

    public String get_process_name() {
        return process_name;
    }

    public int get_process_arrival_time() {
        return process_arrival_time;
    }

    public int get_process_duration() {
        return process_remaining_duration;
    }

    public void print() {
        System.out.println("Proc "+process_name+": Start Time: "+process_arrival_time+", Duration: "+process_original_duration);
    }

    public void setDuration(int dur) {
        if (process_remaining_duration > 0) {
            process_remaining_duration = dur;
        } else process_remaining_duration = 0;
    }
    public void recover_process_original_duration() {
        process_remaining_duration = process_original_duration;
    }
}
