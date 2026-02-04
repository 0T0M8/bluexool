/* =================== TOAST UTILITY =================== */
function showToast(message, type = 'info') {
    const container = document.getElementById('toast-container');
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.textContent = message;
    container.appendChild(toast);
    setTimeout(() => toast.remove(), 3500);
}

/* =================== FORM & TABLE =================== */
const form = document.getElementById('studentForm');
const tableBody = document.getElementById('students');
const searchInput = document.getElementById('search');

/* Reset form */
function resetForm() {
    form.reset();
    document.getElementById('id').value = '';
}

/* =================== LOAD STUDENTS =================== */
async function loadStudents() {
    try {
        const res = await fetch('/students');
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
                <td>${s.status}</td>
                <td>${s.guardian_name}</td>
                <td>${s.contact1}</td>
                <td>${s.contact2}</td>
                <td>
                    <button onclick='editStudent(${JSON.stringify(s)})'>Edit</button>
                    <button class="danger" onclick="deleteStudent(${s.id})">Delete</button>
                </td>
            `;
            tableBody.appendChild(row);
        });
        filterStudents(); // apply current search filter
    } catch (err) {
        showToast(err.message, 'error');
    }
}

/* =================== EDIT STUDENT =================== */
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

/* =================== ADD / UPDATE STUDENT =================== */
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

/* =================== DELETE STUDENT =================== */
async function deleteStudent(id) {
    if (!confirm('Are you sure you want to delete student ID ' + id + '?')) return;
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

/* =================== SEARCH / FILTER =================== */
function filterStudents() {
    const filter = searchInput.value.toLowerCase();
    const rows = tableBody.querySelectorAll('tr');
    rows.forEach(row => {
        const cells = row.querySelectorAll('td');
        const match = Array.from(cells).some(td => td.textContent.toLowerCase().includes(filter));
        row.style.display = match ? '' : 'none';
    });
}
searchInput.addEventListener('input', filterStudents);

/* =================== INITIAL LOAD =================== */
loadStudents();
