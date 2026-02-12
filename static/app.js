/* =================== TOAST UTILITY =================== */
function showToast(message, type = 'info') {
    const container = document.getElementById('toast-container');
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.textContent = message;
    container.appendChild(toast);
    setTimeout(() => toast.remove(), 3500);
}

/* =================== SCREENS =================== */
const loginScreen = document.getElementById('loginScreen');
const registerScreen = document.getElementById('registerScreen');
const appScreen = document.getElementById('appScreen');

function showLoginScreen() {
    loginScreen.style.display = 'block';
    registerScreen.style.display = 'none';
    appScreen.style.display = 'none';
}

function showRegisterScreen() {
    loginScreen.style.display = 'none';
    registerScreen.style.display = 'block';
    appScreen.style.display = 'none';
}

function showAppScreen() {
    loginScreen.style.display = 'none';
    registerScreen.style.display = 'none';
    appScreen.style.display = 'block';
}

/* =================== SESSION CHECK =================== */
async function checkSession() {
    try {
        const res = await fetch('/students');
        if (res.status === 401) {
            showLoginScreen();
        } else {
            showAppScreen();
            loadStudents();
        }
    } catch (err) {
        showLoginScreen();
    }
}

/* =================== LOGIN =================== */
const loginForm = document.getElementById('loginForm');
loginForm.addEventListener('submit', async e => {
    e.preventDefault();
    const username = document.getElementById('loginUsername').value;
    const password = document.getElementById('loginPassword').value;

    try {
        const res = await fetch('/login', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username, password })
        });
        const result = await res.json();
        if (res.status === 200) {
            showToast('Login successful!', 'success');
            showAppScreen();
            loadStudents();
        } else throw new Error(result.error || 'Login failed');
    } catch (err) {
        showToast(err.message, 'error');
    }
});

/* =================== REGISTER =================== */
const registerForm = document.getElementById('registerForm');
registerForm.addEventListener('submit', async e => {
    e.preventDefault();
    const username = document.getElementById('regUsername').value;
    const password = document.getElementById('regPassword').value;

    try {
        const res = await fetch('/register', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username, password })
        });
        const result = await res.json();
        if (res.status === 200 || res.status === 201) {
            showToast('Registration successful! Please login.', 'success');
            showLoginScreen();
        } else throw new Error(result.error || 'Registration failed');
    } catch (err) {
        showToast(err.message, 'error');
    }
});

/* =================== LOGOUT =================== */
const logoutBtn = document.getElementById('logoutBtn');
logoutBtn.addEventListener('click', async () => {
    try {
        await fetch('/logout', { method: 'POST' });
        showToast('Logged out', 'info');
        showLoginScreen();
    } catch (err) {
        showToast('Failed to logout', 'error');
    }
});

/* =================== STUDENT CRUD =================== */
const form = document.getElementById('studentForm');
const tableBody = document.getElementById('students');
const searchInput = document.getElementById('search');
const modal = document.getElementById('studentModal');
const modalBody = document.getElementById('studentDetails');
const modalClose = modal.querySelector('.close');

function resetForm() {
    form.reset();
    document.getElementById('id').value = '';
}

async function loadStudents() {
    try {
        const res = await fetch('/students');
        if (res.status === 401) { 
            showLoginScreen();
            return;
        }
        if (!res.ok) throw new Error('Failed to fetch students');
        const students = await res.json();

        tableBody.innerHTML = '';
        students.forEach(s => {
            const row = document.createElement('tr');
            row.innerHTML = `
                <td>${s.id}</td>
                <td>${s.name}</td>
                <td>${s.gender}</td>
                <td>${s.date_of_birth}</td>
                <td>
                  <span class="status-pill ${s.status === "Active" ? "status-active" : "status-inactive"}">
                  ${s.status}
                  </span>
                </td>
                <td>${s.guardian_name}</td>
                <td>${s.contact1}</td>
                <td>${s.contact2}</td>
                <td>▶</td>
            `;
            row.addEventListener('click', () => showStudentModal(s));
            tableBody.appendChild(row);
        });
        filterStudents();
    } catch (err) {
        showToast(err.message, 'error');
    }
}

/* =================== MODAL & CRUD FUNCTIONS =================== */
function showStudentModal(student) {
    modalBody.innerHTML = `
        <p><strong>ID:</strong> ${student.id}</p>
        <p><strong>Name:</strong> ${student.name}</p>
        <p><strong>Gender:</strong> ${student.gender}</p>
        <p><strong>Date of Birth:</strong> ${student.date_of_birth}</p>
        <p><strong>Status:</strong> ${student.status}</p>
        <p><strong>Guardian:</strong> ${student.guardian_name}</p>
        <p><strong>Contact 1:</strong> ${student.contact1}</p>
        <p><strong>Contact 2:</strong> ${student.contact2}</p>
    `;

    modal.style.display = 'block';
    document.getElementById('editStudentBtn').onclick = () => {
        editStudent(student);
        modal.style.display = 'none';
    };
    document.getElementById('deleteStudentBtn').onclick = async () => {
        if (!confirm(`Delete ${student.name}?`)) return;
        await deleteStudent(student.id);
        modal.style.display = 'none';
        loadStudents();
    };
}

modalClose.onclick = () => modal.style.display = 'none';
window.onclick = e => { if (e.target === modal) modal.style.display = 'none'; };

function editStudent(student) {
    document.getElementById('id').value = student.id;
    document.getElementById('name').value = student.name;
    document.getElementById('gender').value = student.gender;
    document.getElementById('date_of_birth').value = student.date_of_birth;
    document.getElementById('status').value = student.status;
    document.getElementById('guardian_name').value = student.guardian_name;
    document.getElementById('contact1').value = student.contact1;
    document.getElementById('contact2').value = student.contact2;
    showToast('Editing student ID ' + student.id, 'info');
}

/* =================== FORM SUBMIT =================== */
form.addEventListener('submit', async e => {
    e.preventDefault();
    const id = document.getElementById('id').value;
    const data = {
        name: document.getElementById('name').value,
        gender: document.getElementById('gender').value,
        date_of_birth: document.getElementById('date_of_birth').value,
        status: document.getElementById('status').value,
        guardian_name: document.getElementById('guardian_name').value,
        contact1: document.getElementById('contact1').value,
        contact2: document.getElementById('contact2').value
    };
    if (id) data.id = parseInt(id);

    try {
        const res = await fetch('/students', {
            method: id ? 'PUT' : 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        });
        const result = await res.json();
        if (result.error) throw new Error(result.error);

        showToast(id ? 'Student updated!' : 'Student added!', 'success');
        resetForm();
        loadStudents();
    } catch (err) {
        showToast(err.message, 'error');
    }
});

/* =================== DELETE =================== */
async function deleteStudent(id) {
    try {
        const res = await fetch(`/students?id=${id}`, { method: 'DELETE' });
        const result = await res.json();
        if (result.error) throw new Error(result.error);
        showToast('Student deleted!', 'success');
        loadStudents();
    } catch (err) {
        showToast(err.message, 'error');
    }
}

/* =================== SEARCH =================== */
function filterStudents() {
    const filter = document.getElementById('search').value.toLowerCase();
    const rows = tableBody.querySelectorAll('tr');
    rows.forEach(row => {
        const cells = row.querySelectorAll('td');
        const match = Array.from(cells).some(td => td.textContent.toLowerCase().includes(filter));
        row.style.display = match ? '' : 'none';
    });
}
document.getElementById('search').addEventListener('input', filterStudents);

/* =================== INITIALIZATION =================== */
document.addEventListener('DOMContentLoaded', () => {
    checkSession();  // Show login if unauthorized
});
