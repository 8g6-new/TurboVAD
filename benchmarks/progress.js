const {appendFileSync}         = require('fs')

const colors = {
    reset: '\x1b[0m',
    black: '\x1b[30m',
    red: '\x1b[31m',
    green: '\x1b[32m',
    yellow: '\x1b[33m',
    blue: '\x1b[34m',
    magenta: '\x1b[35m',
    cyan: '\x1b[36m',
    white: '\x1b[37m',
    bright_black: '\x1b[90m',
    bright_red: '\x1b[91m',
    bright_green: '\x1b[92m',
    bright_yellow: '\x1b[93m',
    bright_blue: '\x1b[94m',
    bright_magenta: '\x1b[95m',
    bright_cyan: '\x1b[96m',
    bright_white: '\x1b[97m'
};

console.clear = ()=> {
    process.stdout.write('\x1b[2J\x1b[0f');
}

function get_progress(epl, c, total, bs, comp, log="",clear = true,bar_length=40) {
    

    const scc             = comp == bs
    let remaining         = total - c - 1;
    let avg_time_per_item = epl / bs;
    let secs              = avg_time_per_item * remaining / 1000;
    let per               = ((c + 1) * 100) / total;
    let local_end_time    = new Date(Date.now() + secs * 1000).toLocaleString();
    
    const how_much        = `[${comp}/${bs}]`
    const status_symbol   = scc ? `${colors.bright_green}Success ✔ ${how_much} ${colors.reset}` : `${colors.red}❌ Error ${how_much}${colors.reset}`;
    const filled_length   = Math.round(bar_length * per / 100);
    const empty_length    = bar_length - filled_length;

    const progress_bar    = `${colors.bright_blue}[${colors.cyan}${'▰'.repeat(filled_length)}${colors.reset}${colors.bright_blue}${' '.repeat(empty_length)}${colors.reset}${colors.bright_blue}]`;

    const max_label_len   = Math.max(
        "ETA      :".length, 
        "End time :".length, 
        "Process status :".length
    );

    const progress_str    = `${colors.cyan}[${c + 1}/${total}]${colors.reset} ${progress_bar} ${colors.bright_yellow}${per.toFixed(3)}% complete${colors.reset}`;
    const eta_str         = `${colors.blue}ETA${" ".repeat(max_label_len - "ETA".length)}:${colors.reset} ${colors.bright_cyan}${s_hms(secs)}`;
    const speed           = `${epl}  ms for ${bs} fils, ${avg_time_per_item.toFixed(3)} ms per sample) ${colors.bright_yellow}${(1000/avg_time_per_item).toFixed(3)} items per second ${colors.reset}`;
    const status_str      = `${colors.blue}Process status${" ".repeat(max_label_len - "Process status".length)}: ${status_symbol}`;
    const local_time_str  = `${colors.blue}Local time${" ".repeat(max_label_len - "Local time".length)}:${colors.reset} ${colors.magenta}${local_end_time}${colors.reset}`;
    
    const output           = `${progress_str}\n${eta_str}\n${speed}\n${status_str}\n${local_time_str}`

    if(log!=""){
        appendFileSync(log+'.txt', '\n______________START__________________\n'+output+'\n______________END_______________\n');
    }

    if (clear) {
        console.clear();
        process.stdout.cursorTo(0);
        process.stdout.write(`${output}\r\r\r`);
    } else {
        console.log(`${output}`);
    }
}

function s_hms(seconds) {
    const h = Math.floor(seconds / 3600);
    const m = Math.floor((seconds % 3600) / 60);
    const s = Math.floor(seconds % 60);
    return `${h.toString().padStart(2, '0')}:${m.toString().padStart(2, '0')}:${s.toString().padStart(2, '0')}`;
}


module.exports = {
    colors,get_progress,s_hms
}