let data = require('../25_out/benchmarks.json');

// Extend Array prototype for statistics
Array.prototype.mean = function () {
    return this.reduce((a, b) => a + b, 0) / this.length;
};

Array.prototype.sd = function (mean) {
    return Math.sqrt(this.reduce((a, b) => a + Math.pow(b - mean, 2), 0) / this.length);
};

Array.prototype.min = function () {
    return Math.min(...this);
};

Array.prototype.max = function () {
    return Math.max(...this);
};
const unit_map = {
    "elapsed_time": "s",
    "user_time": "s",
    "sys_time": "s",
    "cpu_utilized": "x",
    "task_clock": "",
    "page_faults": "",
    "instructions": "",
    "cycles": "",
    "stalled_cycles_frontend": "",
    "branches": "",
    "branch_misses": "",
    "system_time": "s",
    "ipc": "",
    "branch_miss_%": "%",
    "parallel_speedup": "x",
    "file_read_speed_MB/s": "MB/s",
    "mp3_decode_speed_s/s": "s/s"
};

// List of function times that should be in microseconds
const function_times = [
    ["auto_det","ms",1],
    ["file_read","ms",1],
    ["dec_mp3","ms",1],
    ["init_window_function","ns",1e3],
    ["STFT","ms",1e-3],
    ["norm_all","µs",1],
    ["feat_extraction","µs",1],
    ["pred","ns",1e3],
    ["initialize_mel_filter","ns",1e3],
    ["IO_Loop","ms",1],
    ["total_pro","ms/sec",1e-3]
];

// Add them dynamically to unit_map
function_times.forEach(key => unit_map[key[0]] = key[1]);
const bird_names = Object.keys(data);

// Extract function timing keys and benchmark keys
let fun_times = Object.keys(data[bird_names[0]][0]['function_data']['function_timings']);
let benchmarks = Object.keys(data[bird_names[0]][0]['benchmarks']);

let t = {};

// Initialize empty arrays for function timings and benchmarks
[...fun_times, ...benchmarks].forEach(n => t[n] = []);


t["total_pro"] = []
bird_names.forEach(n => {
    data[n].forEach(entry => {
        // Store function timings

        fun_times.forEach(time_key => {
            t[time_key].push(entry['function_data']['function_timings'][time_key]);
            let total_time =
            (entry['function_data']['function_timings']["STFT"] || 0) +
            (entry['function_data']['function_timings']["norm_all"] || 0) +
            (entry['function_data']['function_timings']["feat_extraction"] || 0) +
            (entry['function_data']['function_timings']["pred"] || 0);

            t["total_pro"].push(total_time);  
        });

        

        // Store benchmark values
        benchmarks.forEach(bench_key => {
            let value = entry['benchmarks'][bench_key];

            // Convert milliseconds to seconds where applicable
            if (bench_key.includes("task_clock")) {
                value /= 1000;
            }
            
            t[bench_key].push(value);
        });
    });
});


benchmarks.forEach(n => {
    t[n] = cal(t[n].filter(n => n)) 
})
// Compute statistics for each key
function_times.forEach(n => {
    t[n[0]] = cal(t[n[0]].filter(n => n),n[2]) 
});


function cal(data,exp=1) {
    if (data.length === 0) return { mean: 0, sd: 0, sd_per: 0, min: 0, max: 0 };

    data = data.map(n=>n*exp)

    const mean = data.mean();
    const sd  = data.sd(mean);
    return {
        "mean": mean,
        "sd": sd,
        "sd_per": (sd * 100) / mean,
        "min": data.min(),
        "max": data.max(),
    };
}

// Format the output with proper units
Object.keys(t).forEach((n, i) => {
    const { mean, sd, min, max,sd_per } = t[n];
    const unit = unit_map[n] || ""; // Use unit map if available, otherwise empty
    try{
        console.log(`${i + 1}) \x1b[36m${n}\x1b[0m`);  // Cyan for function name
        console.log(`   \x1b[32m${formatValue(mean, unit)} ±  ${sdColor(sd, unit)} [${sdColor(sd_per,"")}%] ( min : ${formatValue(min, unit)} max : ${formatValue(max, unit)})\x1b[0m\n`);
    }
    catch(e){}
   
});

// Helper function to color standard deviation
function sdColor(sd, unit) {
    return `\x1b[33m${formatValue(sd, unit)}\x1b[0m`;  // Yellow for SD
}

// Helper function to format numbers
function formatValue(value, unit) {
    if (unit === "%") return value.toFixed(2) + " %";
    if (unit === "x") return value.toFixed(2) + "x";
    if (unit === "MB/s") return value.toFixed(2) + " MB/s";
    if (unit === "s/s") return value.toFixed(2) + " s/s";
    if (unit === "") return value.toFixed(3); // No unit, just a count
    return value.toFixed(3) + " " + unit;
}
