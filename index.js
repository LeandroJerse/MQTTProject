const fs = require('fs')

/*fs.writeFileSync('output.txt', 'E O L, JA FEZ HOJE  ?')
const arquivo = fs.readFileSync('output.txt', 'utf-8');

console.log(arquivo);*/

const moment = require('moment');

const time = new Date();
const timeconverted = moment(time).format('DD/MM/YYYY HH:mm:ss');
console.log(timeconverted);