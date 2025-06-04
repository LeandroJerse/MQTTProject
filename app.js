const express = require('express');
const app = express();
const port = 3000;

app.use(express.json());

let users = [
  { id: 1, name: 'Alice' },
    { id: 2, name: 'Bob' },
    { id: 3, name: 'Charlie' }
];

app.get('/users', (req, res) => {
  res.json(users);
});

app.get('/',(req, res) => {
  res.status(200).send('Welcome to the User API');
});

app.post('/users', (req, res) => {
  if (!req.body || !req.body.name) {
    return res.status(400).json({ message: "O campo 'name' é obrigatório" });
  }
  
  const { name } = req.body;
  const newUser = { id: users.length + 1, name };
  users.push(newUser);
  res.status(201).json(newUser);
});
app.put('/users/:id', (req, res) => {
  const { id } = req.params;
  const { name } = req.body;
  const user = users.find(u => u.id === parseInt(id));
  
  if (!user) {
    return res.status(404).json({ message: 'User not found' });
  }
  
  user.name = name;
  res.json(user);
});
app.delete('/users/:id', (req, res) => {
  const id = parseInt(req.params.id);
  const initialLength = users.length;
  
  users = users.filter(user => user.id !== id);
  
  if (users.length === initialLength) {
    return res.status(404).json({ message: 'Usuário não encontrado' });
  }
  
  res.status(200).json({ message: 'Usuário deletado com sucesso' });
});
app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});