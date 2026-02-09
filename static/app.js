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

/* Modal Elements */
const modal = document.getElementById('studentModal');
const modalBody = document.getElementById('studentDetails');
const modalClose = modal.querySelector('.close');

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

            // Row click opens modal
            row.addEventListener('click', () => showStudentModal(s));
            tableBody.appendChild(row);
        });
        filterStudents();
    } catch (err) {
        showToast(err.message, 'error');
    }
}

/* =================== MODAL FUNCTIONS =================== */
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

    const editBtn = document.getElementById('editStudentBtn');
    editBtn.onclick = () => {
        editStudent(student);
        modal.style.display = 'none';
    };

    const deleteBtn = document.getElementById('deleteStudentBtn');
    deleteBtn.onclick = async () => {
        if (!confirm(`Are you sure you want to delete ${student.name}?`)) return;
        await deleteStudent(student.id);
        modal.style.display = 'none';
        loadStudents();
    };
}

/* Close modal */
modalClose.onclick = () => modal.style.display = 'none';
window.onclick = e => { if (e.target == modal) modal.style.display = 'none'; };

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
