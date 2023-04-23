const projectSearchInput = document.getElementById("projectSearch");
const companySearchInput = document.getElementById("companySearch");
const courseTimeSelect = document.getElementById("coursetime-select");
const primarySelect = document.getElementById("primary-select");
const secondarySelect = document.getElementById("secondary-select");
const tertiarySelect = document.getElementById("tertiary-select");

projectSearchInput.addEventListener("keyup", searchAndFilter);
companySearchInput.addEventListener("keyup", searchAndFilter);
courseTimeSelect.addEventListener("change", searchAndFilter);
primarySelect.addEventListener("change", searchAndFilter);
secondarySelect.addEventListener("change", searchAndFilter);
tertiarySelect.addEventListener("change", searchAndFilter);

function searchAndFilter() {
  var projectCards = document.getElementsByClassName("project-card");
  var projectCardContents = document.getElementsByClassName("project-card-content");
  for (var i = 0; i < projectCards.length; i++) {
    projectCards[i].style.display = "none";
    projectCardContents[i].style.display = "none";
  }
  
  const projectSearchTerm = projectSearchInput.value.toLowerCase();
  const companySearchTerm = companySearchInput.value.toLowerCase();
  var courseTimeFilter = document.getElementById("coursetime-select").value;
  var primaryFilter = document.getElementById("primary-select").value;
  var secondaryFilter = document.getElementById("secondary-select").value;
  var tertiaryFilter = document.getElementById("tertiary-select").value;

  for (var i = 0; i < projectCards.length; i++) {
    const projectName = projectCards[i].querySelector("td:first-child").textContent.toLowerCase();
    const companyName = projectCards[i].querySelector("td:nth-child(2)").textContent.toLowerCase();

    var courseTimeValue = projectCards[i].getElementsByTagName("td")[2].textContent;
    var primaryValue = projectCards[i].getElementsByTagName("td")[3].textContent;
    var secondaryValue = projectCards[i].getElementsByTagName("td")[4].textContent;
    var tertiaryValue = projectCards[i].getElementsByTagName("td")[5].textContent;

    var not_show = projectSearchTerm != "" && !projectName.includes(projectSearchTerm) 
               || companySearchTerm != "" && !companyName.includes(companySearchTerm)
               || courseTimeFilter !== "" && !courseTimeValue.includes(courseTimeFilter)
               || primaryFilter !== "" && primaryValue !== primaryFilter
               || secondaryFilter !== "" && secondaryValue !== secondaryFilter
               || tertiaryFilter !== "" && !tertiaryValue.includes(tertiaryFilter);

    if (not_show) {
      projectCards[i].style.display = "none";
    } else {
      projectCards[i].style.display = "";
    }
  }
}

function toggleCard(element) {
  var content = element.nextElementSibling;

  element.classList.toggle("active");

  if (content && content.style.display === "none") {
    content.style.display = "table-row";
    var students = content.getElementsByClassName("student-names")[0];
    if (students) {
      students.style.display = "block";
    }
  } else {
    content.style.display = "none";
    var students = content.getElementsByClassName("student-names")[0];
    if (students) {
      students.style.display = "none";
    }
  }
}


async function getProjectInstructorMap() {
  var projectInstructorMap = {};
  const response = await fetch("/project_instructor_map");
  const data = await response.json();
  Object.keys(data).forEach(key => {
      projectInstructorMap[key] = data[key];
  });

  return projectInstructorMap;
}

async function getProjectData() {
  const tableBody = document.getElementById("project-info");
  const projectInstructorMap = await getProjectInstructorMap();

  const response = await fetch("/project_json");
  const data = await response.json();

  for (const key of Object.keys(data)) {
    const val = data[key];
    const tbody = document.querySelector("#project-info");
    const template = document.querySelector('#project-card-temp');

    const clone = template.content.cloneNode(true);
    let td = clone.querySelectorAll("td");

    // set project-card
    td[0].textContent = val.projectTitle;
    td[1].textContent = val.companyName;
    td[2].textContent = val.courseTime;
    td[3].textContent = val.firstPreference;
    td[4].textContent = val.secondPreference;
    td[5].textContent = val.thirdPreference;

    // set project-card-content
    td[8].textContent = val.courseName;
    td[9].textContent = val.courseTime;
    td[10].textContent = projectInstructorMap[key];
    td[11].textContent = val.ip;
    td[12].textContent = val.confidentiality;
    td[13].textContent = val.physicalPrototype;

    const assignmentResponse = await fetch("/students_under_project?projectID=" + String(key));
    const assignmentData = await assignmentResponse.json();

    td[6].textContent = assignmentData == null? "0" : Object.keys(assignmentData).length;

    const assignemntTbody= clone.getElementById("assignementTbody");

    if (assignmentData != null) {
        for (const campusID of Object.keys(assignmentData)) {
        const assignmentVal = assignmentData[campusID];
        const assignemntTemplate = document.querySelector('#assignemntTemplate');
        const assignmentClone = assignemntTemplate.content.cloneNode(true);
        let assignment_td = assignmentClone.querySelectorAll("td");

        assignment_td[1].textContent = assignmentVal["FIRSTNAME"];
        assignment_td[2].textContent = assignmentVal["LASTNAME"];
        assignment_td[3].textContent = assignmentVal["MAJOR"];
        assignment_td[4].textContent = assignmentVal["PREFERRED"];
        assignment_td[5].textContent = assignmentVal["COMMENT"];

        assignemntTbody.appendChild(assignmentClone);
        assignemntTbody.id = "assignemntTbody" + campusID;
      }
    }

      const button = clone.getElementById("customizeMenu");
      button.id = "button-" + key;
      button.setAttribute("data-bs-target", "#"+key);

      const allAssignemntsTbody= clone.getElementById("allAssignementsTbody");
      allAssignemntsTbody.id = "allAssignementsTbody-" + button.id;

      button.addEventListener("click", async function () {
        const allAssignmentsResponse = await fetch("/assignments");
        const allAssignmentsData = await allAssignmentsResponse.json();

        if (allAssignmentsData != null) {
          for (const allAssignmentsCampusID of Object.keys(allAssignmentsData)) {
            const allAssignmentsVal = allAssignmentsData[allAssignmentsCampusID];
            const allAssignemntsTbody= document.querySelector("#allAssignementsTbody-"+button.id);
            const allAssignmentsTemplate = document.querySelector('#allAssignemntsTemplate');
            const allAssignmentsClone = allAssignmentsTemplate.content.cloneNode(true);
            let all_assignments_td = allAssignmentsClone.querySelectorAll("td");     

            all_assignments_td[0].textContent = allAssignmentsVal["FIRSTNAME"];
            all_assignments_td[1].textContent = allAssignmentsVal["LASTNAME"];
            all_assignments_td[2].textContent = allAssignmentsVal["MAJOR"];
            all_assignments_td[3].textContent = allAssignmentsVal["PREFERRED"];
            all_assignments_td[4].textContent = allAssignmentsVal["COMMENT"];

            allAssignemntsTbody.appendChild(allAssignmentsClone);
          }
        }
    })


    const modal = clone.getElementById("staticBackdrop");
    modal.id = key;

    tbody.appendChild(clone);
  }
}
