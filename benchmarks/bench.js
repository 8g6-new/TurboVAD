const {readdir,writeFile,mkdir}         = require('fs/promises');

const {colors,get_progress,s_hms} = require('./progress')
const color_sch                   = require('./colors.json')
let   { exec } = require('child_process');
const  {promisify}     =  require('util');

exec = promisify(exec);

function parseLog(log,length) {
    
    log = log.split('\n\n')
    let out = {};

    let fileMatch = log[0].match(/(.+\.mp3) auto detected to be (audio\/\w+)/);
    if (fileMatch) {
        out.filePath = fileMatch[1];
        out.format = fileMatch[2];
    }

    let keys = ["duration", "channels", "num_samples", "sample_rate", "file_size_bytes"];
    keys.forEach(key => {
        let match = log[1].match(new RegExp(`${key}:(\\d+\\.?\\d*)`));
        if (match) {
            out[key] = match[1].includes('.') ? parseFloat(match[1]) : parseInt(match[1]);
        }
    });


    let function_timings = {};
    let matches = [...log[3].matchAll(/(\w+):(\d+)/g)];
    matches.forEach(match => {
        function_timings[match[1]] = parseInt(match[2]);
    });

    let data = Object.keys(function_timings)

    data.forEach(n=>{
        function_timings[n]/=(n!='segments' ? function_timings['segments'] : 1)*length
    })

    return {
        "file_properties": out,
        "function_timings": function_timings
    };
}


const regexPatterns = {
    elapsed_time: /([\d.]+)\s+seconds time elapsed/,
    user_time: /([\d.]+)\s+seconds user/,
    sys_time: /([\d.]+)\s+seconds sys/,
    cpu_utilized: /([\d.]+)\s+CPUs utilized/,
    task_clock: /([\d.]+)\s+msec task-clock:u/,
    page_faults: /([\d,]+)\s+page-faults:u/,
    instructions: /([\d,]+)\s+instructions:u/,
    cycles: /([\d,]+)\s+cycles:u/,
    stalled_cycles_frontend: /([\d,]+)\s+stalled-cycles-frontend:u/,
    branches: /([\d,]+)\s+branches:u/,
    branch_misses: /([\d,]+)\s+branch-misses:u/,
    system_time: /([\d.]+)\s+seconds user/
};

function parse_perf_output(output) {
    let results = {};

    for (let key in regexPatterns) {
        let match = output.match(regexPatterns[key]);
        if (match) {
            let value = match[1].replace(/,/g, '');
            results[key] = parseFloat(value);
        } else {
            results[key] = null;
        }
    }

    results['ipc'] = results['instructions']/results['cycles']

    if (results["branches"] && results["branch_misses"]) {
        results["branch_miss_%"] = (results["branch_misses"] * 100) / results["branches"];
    }

    if (results["system_time"] && results["elapsed_time"]) {
        results["parallel_speedup"] = results["system_time"] / results["elapsed_time"];
    }

    return results;
}


let spcs = {
    "input": '/home/dsb/disks/data/paper/c/c_spectrogram/tests/files/Voice of Birds/ana/det/audio/25/Blue Jay/1.mp3',
    "window_size_pred": 1024,
    "window_size_img": 512,
    "hop_size_pred": 128,
    "hop_size_img": 512,
    "window_type": "hann",
    "window_type": "hann",
    "seg_length": 0.5,
    "num_mel": 256,
    "min_mel_freq": 0,
    "max_mel_freq": 7500,
    "output_wav": "./out/wav",
    "output_stft": "./out/stft/",
    "output_mel": "./out/mel/",
    "cache_dir": "./cache/FFT",
    "color": {
        "stft":color_sch.builtin.Spectral.soft,"mel":color_sch.builtin.Blues.soft
    },
    "th": 0.001
}

function get_rand(array, n) {
    if (n > array.length) {
        n=array.length
    }

    const result = new Set();
    while (result.size < n) {
        const idx= Math.floor(Math.random() * array.length);
        result.add(array[idx]);
    }

    return Array.from(result);
}



async function conv(spcs,length) {
    let command = `perf stat ./builtin "${spcs.input}" ${spcs.window_size_pred} ${spcs.window_size_img} ${spcs.hop_size_pred} ${spcs.hop_size_img} ${spcs.window_size_pred} ${spcs.window_size_img} ${spcs.seg_length} ${spcs.num_mel} ${spcs.min_mel_freq} ${spcs.max_mel_freq} ${spcs.output_wav} ${spcs.output_stft} ${spcs.output_mel} ${spcs.cache_dir} ${spcs.color.stft} ${spcs.color.mel} ${spcs.th}`;

    let { stderr, stdout } = await exec(command);

    let fun        = parseLog(stdout,length);
    let benchmarks = parse_perf_output(stderr);

    let fileSize     = fun.file_properties.file_size_bytes || 0;
    let fileReadTime = fun.function_timings.file_read || fun.function_timings.dec_wav; 
    let decodeTime   = fun.function_timings.dec_mp3   || fun.function_timings.dec_wav;

    benchmarks["file_read_speed_MB/s"] = (1.048576 * fileSize) / fileReadTime;
    benchmarks["mp3_decode_speed_s/s"] = fun.file_properties.duration * 1e6/ decodeTime;

    return {
        "function_data": fun,
        "benchmarks": benchmarks
    };
}



function chunks_maker(arr, n) {
    let result = [];
    for (let i = 0; i < arr.length; i += n) {
        result.push(arr.slice(i, i + n));
    }
    return result;
}

Array.prototype.sum = function (){
    return this.reduce((a,b)=>a+b,0)
}
async function run_all(inp,out,spcs,num_t,cs) {

    await mkdir(out,{recursive:true})
    
    let folders = await readdir(inp)

    let files = await Promise.all(folders.map(async(bird)=>{
        let files = await readdir(`${inp}/${bird}`)
        files     = files.filter(n=>n.endsWith('.mp3'))
        return [bird, files]
    }))


    let total = files.map(n=>num_t).sum()


    let  benchmarks   = {}
    let  multi_file_b = {}
    let avg           = 0;

    let c= 0


    for(let [bird_name,fs] of files){

        fs = get_rand(fs,num_t)

        benchmarks[bird_name] = []
        multi_file_b[bird_name]= []

        let chunks = chunks_maker(fs,cs)

        let t = 0;
    
        for(chunk of chunks){

            multi_file_b[bird_name][t] = {
                "files": [],
                "benchmark":[]
            }
      
           let s = performance.now()

            let results = await Promise.allSettled(chunk.map(async(f)=>{
               spcs.input = `${inp}/${bird_name}/${f}`

               return  await conv(spcs,spcs['seg_length'])
            }))

            let e = performance.now()


            results = results.filter(result => result.status === 'fulfilled').map(result => result.value);  
              
            try{
    
                benchmarks[bird_name] = benchmarks[bird_name].concat(results)
    
                avg+=(e-s)

                multi_file_b[bird_name][t]['files'].push(results.map(n=>n["function_data"]['file_properties']['filePath']))
                multi_file_b[bird_name][t]["benchmark"].push(e-s)
                
                t++;
                get_progress(
                    avg/c,
                    c,
                    total,
                    chunk.length,
                    results.length,
                    "",
                    true)
                c+=chunk.length
            }
            catch(e){
                console.error(e)
            }

        }
    }
    
    await Promise.all([
        ["benchmarks",benchmarks],
        ["multi_file_b",multi_file_b],
    ].map(async(n)=>{
        await writeFile(`${out}/${n[0]}.json`, JSON.stringify(n[1],null,2))
    }))
  
}


(async()=>{
    await run_all(
        "/home/dsb/disks/data/paper/c/c_spectrogram/tests/files/Voice of Birds/ana/det/audio/25/",
        "./25_out",
        spcs,6,1)

    // await run_all(
    //         "/home/dsb/disks/data/paper/c/c_spectrogram/tests/files/Voice of Birds/ana/det/audio/Voice of Birds/",
    //         "./Voice of Birds",
    //         spcs,5,5)
    // await run_all(
    //         "/home/dsb/disks/data/paper/c/c_spectrogram/tests/files/Voice of Birds/ana/det/audio/iBC53/",
    //         "./iBC53/",
    //         spcs,2,2)

})()