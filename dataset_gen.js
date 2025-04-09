const {readdir,writeFile} = require('fs/promises');

function concat_json(a,b) {
    const labels = Object.keys(a)
    
    labels.forEach(n=>{
        a[n] = a[n].concat(b[n])
    })

    return a
}

async function conv(data_dir,labels,y){

    labels  = labels.split(',')
    let out = {}

    let data = await readdir(data_dir);
    
    data = data.map(n=>n.split("_")[1].split(',').map(n=>parseFloat(n)))

    labels.forEach(n=>{
        out[n]=[]
    })

    out['bird'] = []

    data.forEach(n=>{
        try{
            n.forEach((a,i)=>{
                out[labels[i]].push(a)
            })
        }
        catch(e){

        }
        out['bird'].push(y)    
    })

    return out
}

async function run(fol,labels){
    const out = await Promise.all([
        await conv(fol+'bird',labels,1),
        await conv(fol+'nobird',labels,0)
    ])
    const joined = concat_json(out[0],out[1])
    await writeFile("dataset.json",JSON.stringify(joined,null,2))
}

run('/home/dsb/disks/data/paper/c/mean/data/aa/out/stft/','Time_Start,Time_End,Spectral_Centroid,Spectral_Centroid_Standard_Deviation,Spectral_Entropy,Spectral_Entropy_Standard_Deviation,Spectral_Flatness,Spectral_Flatness_Standard_Deviation,Harmonic_To_Noise_Ratio,HNR_Standard_Deviation')
