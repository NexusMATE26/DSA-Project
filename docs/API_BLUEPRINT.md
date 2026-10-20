# Planned local JSON API — NOT IMPLEMENTED IN M0

- GET  /api/health
- GET  /api/skills
- GET  /api/companies
- GET  /api/internships?search=&sort=&from=&to=
- POST /api/students
- GET  /api/students/{studentId}
- PUT  /api/students/{studentId}/skills/{skillId}
- POST /api/students/{studentId}/undo
- GET  /api/students/{studentId}/recommendations
- GET  /api/students/{studentId}/plans/{internshipId}
- POST /api/applications
- POST /api/applications/{applicationId}/withdraw
- POST /api/admin/skills
- POST /api/admin/companies
- POST /api/admin/internships
- POST /api/admin/skill-prerequisites
- POST /api/admin/internships/{internshipId}/close

Each endpoint must validate inputs and use AppState/services. The transport
adapter must not duplicate matching, sorting, or graph logic. Local prototype
uses a demo profile selector, NOT real authenticated accounts. Never deploy it
on the public internet in this form.
